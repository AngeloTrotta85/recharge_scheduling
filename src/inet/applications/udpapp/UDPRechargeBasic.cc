//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "UDPRechargeBasic.h"

#include <stdio.h>
#include <stdlib.h>

#include <iomanip>      // std::setprecision

#include "inet/networklayer/common/L3AddressResolver.h"

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/transportlayer/contract/udp/UDPDataIndicationExt_m.h"

#include "inet/common/geometry/common/Coord.h"

namespace inet {

Define_Module(UDPRechargeBasic)

UDPRechargeBasic::~UDPRechargeBasic() {
    cancelAndDelete(autoMsgRecharge);
    cancelAndDelete(stat1sec);
    cancelAndDelete(stat5sec);
    cancelAndDelete(goToCharge);
}

void UDPRechargeBasic::initialize(int stage)
{
    UDPBasicApp::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        mob = check_and_cast<VirtualSpringMobility *>(this->getParentModule()->getSubmodule("mobility"));
        sb = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getSubmodule("battery"));

        myAppAddr = this->getParentModule()->getIndex();

        checkRechargeTimer = par("checkRechargeTimer");
        sensorRadious = par("sensorRadious");
        chargingStationNumber = par("chargingStationNumber");

        double rx = (mob->getConstraintAreaMax().x - mob->getConstraintAreaMin().x) / 2.0;
        double ry = (mob->getConstraintAreaMax().y - mob->getConstraintAreaMin().y) / 2.0;
        rebornPos = Coord(rx, ry);

        rechargeLostAccess = 0;
        failedAttemptCount = 0;

        activeNodesVector.setName("activeNodes");
        rechargingNodesVector.setName("rechargingNodes");
        responseVector.setName("ResponseVal");
        degreeVector.setName("DegreeVal");
        energyVector.setName("EnergyVal");
        failedAttemptVector.setName("FailedAttemptVal");
        dischargeProbVector.setName("DischargeProbVal");

        goToCharge = new cMessage("goToCharge");

        stat1sec = new cMessage("stat1secMsg");
        scheduleAt(simTime(), stat1sec);

        stat5sec = new cMessage("stat5secMsg");
        scheduleAt(simTime(), stat5sec);

        autoMsgRecharge = new cMessage("msgRecharge");
        scheduleAt(simTime() + checkRechargeTimer, autoMsgRecharge);
    }
    else if (stage == INITSTAGE_LAST) {
        myAddr = L3AddressResolver().resolve(this->getParentModule()->getFullPath().c_str());
        this->getParentModule()->getDisplayString().setTagArg("t", 0, myAddr.str().c_str());

        sb->setState(power::SimpleBattery::DISCHARGING);
    }
}

void UDPRechargeBasic::finish(void) {
    if (myAppAddr == 0) {

        int numberNodes = this->getParentModule()->getVectorSize();

        double sumEnergy = 0.0;
        for (int i = 0; i < numberNodes; i++) {
            power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", i)->getSubmodule("battery"));
            sumEnergy += battN->getBatteryLevelAbs();
        }
        recordScalar("FINALENERGY", sumEnergy);
        recordScalar("LIFETIME", simTime());
    }

    recordScalar("FAILED_ATTEMPT_COUNT", failedAttemptCount);
    recordScalar("FAILED_ATTEMPT_FREQ", ((double)failedAttemptCount)/simTime().dbl());
}


void UDPRechargeBasic::handleMessageWhenUp(cMessage *msg) {
    if (msg == autoMsgRecharge) {
        if (sb->getState() == power::SimpleBattery::CHARGING){
            bool stopCharging = checkDischarge();

            if (stopCharging) {

                //stop the node
                neigh.clear();
                //mob->clearVirtualSprings();
                mob->clearVirtualSpringsAndsetPosition(rebornPos);

                sb->setState(power::SimpleBattery::DISCHARGING);

                // reschedule now autoMsgRecharge to check now if return to recharge
                scheduleAt(simTime(), autoMsgRecharge);
                return;
            }
        }
        else if (sb->getState() == power::SimpleBattery::DISCHARGING){
            bool toCharge = checkRecharge();

            if (toCharge) {
                double backoff = calculateSendBackoff();

                sendRechargeMessage();
                scheduleAt(simTime() + backoff, goToCharge);
            }
        }

        scheduleAt(simTime() + checkRechargeTimer, autoMsgRecharge);
    }
    else if (msg == goToCharge) {

        if (checkRechargingStationFree()) {

            rechargeLostAccess = 0;

            sb->setState(power::SimpleBattery::CHARGING);

        }
        else {
            rechargeLostAccess++;
            failedAttemptCount++;

            sb->setDoubleSwapPenality();
        }

        //stop the node
        neigh.clear();
        //mob->clearVirtualSprings();
        mob->clearVirtualSpringsAndsetPosition(rebornPos);

    }
    else if (msg == stat5sec) {
        make5secStats();
        scheduleAt(simTime() + 5, msg);
    }
    else if (msg == stat1sec) {
        updateNeighbourhood();
        updateVirtualForces();

        make1secStats();
        scheduleAt(simTime() + 1, msg);
    }
    else {
        UDPBasicApp::handleMessageWhenUp(msg);
    }
}

bool UDPRechargeBasic::checkRechargingStationFree(void) {
    bool ris = true;
    int numberNodes = this->getParentModule()->getVectorSize();
    int nodesInCharging = 0;

    for (int i = 0; i < numberNodes; i++) {
        power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", i)->getSubmodule("battery"));

        if (battN->isCharging()) {
            nodesInCharging++;
        }
    }

    if (nodesInCharging >= chargingStationNumber) {
        ris = false;
    }

    return ris;
}

void UDPRechargeBasic::processPacket(cPacket *pk)
{
    //EV << "RECEIVED PACKET: " << pk->getName() << endl;
    if (sb->getState() == power::SimpleBattery::DISCHARGING) {
        ApplicationPacketRecharge *aPkt = check_and_cast<ApplicationPacketRecharge *> (pk);
        if (myAddr != aPkt->getAddr()) {

            cObject *c = pk->getControlInfo();
            UDPDataIndicationExt *di = check_and_cast<UDPDataIndicationExt *>(c);

            //EV_DEBUG << "Received recharge packet " << aPkt->getName() << " with " << di->getFullName() << endl;

            if (aPkt->getGoingToRecharge()) {
                if (neigh.count(aPkt->getAppAddr()) != 0) {
                    neigh.erase(aPkt->getAppAddr());
                }
            }
            else {

                if (neigh.count(aPkt->getAppAddr()) == 0) {
                    nodeInfo_t newInfo;
                    newInfo.addr = aPkt->getAddr();
                    newInfo.appAddr = aPkt->getAppAddr();

                    neigh[aPkt->getAppAddr()] = newInfo;
                }

                nodeInfo_t *node = &(neigh[aPkt->getAppAddr()]);
                node->pos = aPkt->getPos();
                node->rcvPow = di->getPow();
                node->rcvSnr = di->getSnr();

                node->timestamp = simTime();

                node->batteryLevelAbs = aPkt->getBatteryLevelAbs();
                node->batteryLevelPerc = aPkt->getBatteryLevelPerc();
                node->coveragePercentage = aPkt->getCoveragePercentage();
                node->leftLifetime = aPkt->getLeftLifetime();
                node->nodeDegree = aPkt->getNodeDegree();
                node->inRechargeT = aPkt->getInRecharge();
                node->gameTheoryC = aPkt->getGameTheoryC();
            }

            updateVirtualForces();

            EV << "[" << myAppAddr << "] - NEW PACKET arrived. Neigh size: " << neigh.size() << endl;
        }

        emit(rcvdPkSignal, pk);
        //EV_INFO << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
        numReceived++;
    }

    delete pk;
}

ApplicationPacketRecharge *UDPRechargeBasic::generatePktToSend(const char *name, bool goingToRecharge) {
    ApplicationPacketRecharge *payload = new ApplicationPacketRecharge(name);
    payload->setByteLength(par("messageLength").longValue());
    payload->setSequenceNumber(numSent);

    payload->setGoingToRecharge(goingToRecharge);

    payload->setPos(mob->getCurrentPosition());
    payload->setAddr(myAddr);
    payload->setAppAddr(myAppAddr);
    payload->setBatteryLevelAbs(sb->getBatteryLevelAbs());
    payload->setBatteryLevelPerc(sb->getBatteryLevelPerc());
    payload->setCoveragePercentage(0);
    payload->setLeftLifetime(sb->getBatteryLevelAbs() / sb->getDischargingFactor(checkRechargeTimer));
    payload->setNodeDegree(calculateNodeDegree());
    payload->setInRecharge(0); //TODO
    payload->setGoingToRechargeTime(calculateRechargeTime());
    payload->setGameTheoryC(getGameTheoryC());

    return payload;
}

void UDPRechargeBasic::sendRechargeMessage(void) {
    std::ostringstream str;
    str << packetName << "-RECHARGE-" << numSent;

    ApplicationPacketRecharge *payload = generatePktToSend(str.str().c_str(), true);

    L3Address destAddr = chooseDestAddr();
    emit(sentPkSignal, payload);
    socket.sendTo(payload, destAddr, destPort);
    numSent++;
}


void UDPRechargeBasic::sendPacket()
{
    if (sb->getState() == power::SimpleBattery::DISCHARGING) {
        std::ostringstream str;
        str << packetName << "-" << numSent;

        ApplicationPacketRecharge *payload = generatePktToSend(str.str().c_str(), false);

        L3Address destAddr = chooseDestAddr();
        emit(sentPkSignal, payload);
        socket.sendTo(payload, destAddr, destPort);
        numSent++;
    }
}

double UDPRechargeBasic::calculateSendBackoff(void){
    double cw = 0.5;
    double ris = 0;

    //cw = cw / (((double)rechargeLostAccess) + 1.0);
    cw = cw / pow(2.0, ((double) rechargeLostAccess));

    ris = cw * dblrand();
    ris += 0.001;

    return ris;

}


double UDPRechargeBasic::calculateInterDistance(double radious) {
    return (sqrt(3.0) * radious);
}

void UDPRechargeBasic::updateVirtualForces(void) {
    Coord myPos = mob->getCurrentPosition();

    // clear everything
    mob->clearVirtualSprings();

    for (auto it = neigh.begin(); it != neigh.end(); it++) {
        nodeInfo_t *act = &(it->second);
        double preferredDistance = calculateInterDistance(sensorRadious);

        double distance = act->pos.distance(myPos);
        double springDispl = preferredDistance - distance;

        Coord uVec = Coord(1, 1);
        if (distance == 0)  uVec = Coord(dblrand(), dblrand());
        else  uVec = act->pos - myPos;
        uVec.normalize();

        //EV << "Setting force with displacement: " << springDispl << " (distance: " << distance << ")" << endl;
        //fprintf(stderr, "[%d] - adding force with displacement %lf\n", myAppAddr, springDispl);fflush(stderr);
        mob->addVirtualSpring(uVec, preferredDistance, springDispl);
    }

    // add the force towards the center rebornPos
    if (rebornPos.distance(myPos) > 10) {
        Coord uVec = rebornPos - myPos;
        //Coord uVec = myPos - rebornPos;
        uVec.normalize();
        mob->addVirtualSpring(uVec, rebornPos.distance(myPos), -3);
    }
}

void UDPRechargeBasic::updateNeighbourhood(void) {
    bool removed;
    do {
        removed = false;
        for (auto it = neigh.begin(); it != neigh.end(); it++) {
            nodeInfo_t *act = &(it->second);

            if ((simTime() - act->timestamp) > (2.0 * par("sendInterval").doubleValue())) {

                EV << "[" << myAppAddr << "] - UPDATE NEIGHBOURHOOD. Removing: " << it->second.appAddr << endl;

                neigh.erase(it);
                removed = true;
                break;
            }
        }
    } while(removed);
}

void UDPRechargeBasic::getFilteredNeigh(std::map<int, nodeInfo_t> &filteredNeigh){
    std::list<VirtualSpringMobility::NodeBasicInfo> nodesToFilter;
    std::list<VirtualSpringMobility::NodeBasicInfo> nodeFiltered;

    filteredNeigh.clear();

    for (auto it = neigh.begin(); it != neigh.end(); it++) {
        VirtualSpringMobility::NodeBasicInfo newinfo;
        newinfo.id = it->first;
        newinfo.position = it->second.pos;
        nodesToFilter.push_back(newinfo);
    }
    mob->filterNodeListAcuteAngleTest(nodesToFilter, nodeFiltered);

    for (auto it = neigh.begin(); it != neigh.end(); it++) {
        nodeInfo_t *act = &(it->second);

        for (auto it2 = nodeFiltered.begin(); it2 != nodeFiltered.end(); it2++) {
            if (act->appAddr == it2->id) {
                filteredNeigh[it->first] = it->second;
                break;
            }
        }
    }
}

int UDPRechargeBasic::calculateNodeDegree(void) {
    std::map<int, nodeInfo_t> filteredNeigh;
    getFilteredNeigh(filteredNeigh);
    return filteredNeigh.size();
}

double UDPRechargeBasic::calculateRechargeProb(void){
    return 0.25;
}

double UDPRechargeBasic::calculateDischargeProb(void){
    return 1.0;
}

bool UDPRechargeBasic::checkRecharge(void) {
    return (dblrand() < calculateRechargeProb());
}

bool UDPRechargeBasic::checkDischarge(void) {
    return (dblrand() < calculateDischargeProb());
}

void UDPRechargeBasic::make5secStats(void) {

}

void UDPRechargeBasic::make1secStats(void) {

    if (myAppAddr == 0) {
        int nnodesActive = 0;
        int nnodesRecharging = 0;

        int numberNodes = this->getParentModule()->getVectorSize();

        for (int i = 0; i < numberNodes; i++) {
            power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", i)->getSubmodule("battery"));

            if (battN->isCharging()) {
                nnodesRecharging++;
            }
            else {
                nnodesActive++;
            }
        }

        activeNodesVector.record(nnodesActive);
        rechargingNodesVector.record(nnodesRecharging);
    }


    degreeVector.record(calculateNodeDegree());
    failedAttemptVector.record(failedAttemptCount);
    if (sb->isCharging()) {
        dischargeProbVector.record(calculateDischargeProb());
    }
    else {
        responseVector.record(calculateRechargeProb());
    }

    energyVector.record(sb->getBatteryLevelAbs());
}

} /* namespace inet */
