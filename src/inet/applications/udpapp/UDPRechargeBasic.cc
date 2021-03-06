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
    cancelAndDelete(backAfterLoose);
    cancelAndDelete(posMessage);
}

void UDPRechargeBasic::initialize(int stage)
{
    UDPBasicApp::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        mob = check_and_cast<VirtualSpringMobility *>(this->getParentModule()->getSubmodule("mobility"));
        sb = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getSubmodule("battery"));

        myAppAddr = this->getParentModule()->getIndex();
        numberNodesInSimulation = this->getParentModule()->getVectorSize();

        checkRechargeTimer = par("checkRechargeTimer");
        sensorRadious = par("sensorRadious");
        chargingStationNumber = par("chargingStationNumber");
        activateVirtualForceMovements = par("activateVirtualForceMovements").boolValue();
        rechargeIsolation = par("rechargeIsolation").boolValue();
        makeCoverageLog = par("makeCoverageLog").boolValue();
        makeCoverageMap = par("makeCoverageMap").boolValue();
        coverageMapFilename = par("coverageMapFilename").stdstringValue();
        flightHeight = par("flightHeight");
        sensorAngle = par("sensorAngle");
        sendDifferentMessages = par("sendDifferentMessages").boolValue();
        positionMessageTimer = par("positionMessageTimer");
        delayTimeToUpdateEnergy = par("delayTimeToUpdateEnergy");
        useEnergyToShare = par("useEnergyToShare").boolValue();
        shift5secTimer = par("shift5secTimer").boolValue();

        timeToUpdateEnergy = simTime() + delayTimeToUpdateEnergy;

        double rx = (mob->getConstraintAreaMax().x - mob->getConstraintAreaMin().x) / 2.0;
        double ry = (mob->getConstraintAreaMax().y - mob->getConstraintAreaMin().y) / 2.0;
        rebornPos = Coord(rx, ry);

        rechargeLostAccess = 0;
        failedAttemptCount = 0;
        totalAttemptCount = 0;
        inRechargingTime = 0;
        startRecharge = simTime();
        lastSawInRecharge = simTime();
        saveNeighboursMsgs = true;
        sumCoverageTot = sumCoverageRatioTot = sumCoverageRatioMaxTot = countCoverage = 0.0;
        coverageMapIdx = 0;

        looseRechargingChance = false;

        activeNodesVector.setName("activeNodes");
        rechargingNodesVector.setName("rechargingNodes");
        responseVector.setName("ResponseVal");
        degreeVector.setName("DegreeVal");
        fulldegreeVector.setName("FullDegreeVal");
        energyVector.setName("EnergyVal");
        energyVectorAllMean.setName("EnergyVectorAllMean");
        energyVectorAllMin.setName("EnergyVectorAllMin");
        energyVectorAllMax.setName("EnergyVectorAllMax");
        energyVectorAllMaxMinDiff.setName("energyVectorAllMaxMinDiff");
        energyVectorAllVar.setName("EnergyVectorAllVar");
        failedAttemptVector.setName("FailedAttemptVal");
        dischargeProbVector.setName("DischargeProbVal");
        timeOfRechargeVector.setName("TimeOfRechargeVector");
        hypotheticalDischargeProbVector.setName("HypotheticalDischargeProbVector");
        hypotheticalResponseVector.setName("HypotheticalResponseVector");
        totalCoverageVector.setName("totalCoverageVector");;
        totalCoverageRatioVector.setName("totalCoverageRatioVector");
        totalCoverageRatioMaxVector.setName("totalCoverageRatioMaxVector");

        for (int i = 0; i < N_PERCENTAGE_COVERAGE; i++) {
            firstCoveragePassPercent[i] = false;
            endCoveragePassPercent[i] = 0;
            endCoveragePassPercent2[i] = 0;
            endCoveragePassPercent3[i] = 0;
            numberCoveragePassPercent[i] = 0;
        }


        goToCharge = new cMessage("goToCharge");
        backAfterLoose = new cMessage("backAfterLoose");

        stat1sec = new cMessage("stat1secMsg");
        scheduleAt(simTime(), stat1sec);

        stat5sec = new cMessage("stat5secMsg");
        if (shift5secTimer) {
            scheduleAt(simTime() + 0.5, stat5sec);
        }
        else {
            scheduleAt(simTime(), stat5sec);
        }

        autoMsgRecharge = new cMessage("msgRecharge");
        scheduleAt(simTime() + checkRechargeTimer, autoMsgRecharge);

        posMessage = new cMessage("posMessage");
        if (sendDifferentMessages) {
            scheduleAt(simTime() + positionMessageTimer + dblrand(), posMessage);
        }
    }
    else if (stage == INITSTAGE_LAST) {
        myAddr = L3AddressResolver().resolve(this->getParentModule()->getFullPath().c_str());
        this->getParentModule()->getDisplayString().setTagArg("t", 0, myAddr.str().c_str());

        sb->setState(power::SimpleBattery::DISCHARGING);
        energyAtRecharge = sb->getBatteryLevelAbs();

        energyToShare = sb->getBatteryLevelAbs();

        //double maxArea = ((double) numberNodes) * ((sensorRadious*sensorRadious) * (3.0 / 2.0) * sqrt(3.0));
        //double maxArea = ((double) numberNodes) * ((sensorRadious*sensorRadious) * 2.598076211);
        //double maxArea = ((double) (numberNodes - chargingStationNumber)) * ((sensorRadious*sensorRadious) * 2.598076211);
        sensorRadious = flightHeight * tan(sensorAngle/2.0);
        areaMaxToCoverage = ((double) (numberNodesInSimulation - chargingStationNumber)) * ((sensorRadious*sensorRadious) * 2.598076211);
    }
}

void UDPRechargeBasic::finish(void) {
    if (myAppAddr == 0) {

        int numberNodes = this->getParentModule()->getVectorSize();
        double maxEnergy = 0.0;
        double sumEnergy = 0.0;
        double meanEnergy, varEnergy;

        for (int i = 0; i < numberNodes; i++) {
            power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", i)->getSubmodule("battery"));
            double actE = battN->getBatteryLevelAbs();
            sumEnergy += actE;

            if (maxEnergy < actE) maxEnergy = actE;
        }
        meanEnergy = sumEnergy / ((double)numberNodes);

        double sumVar = 0.0;
        for (int i = 0; i < numberNodes; i++) {
            power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", i)->getSubmodule("battery"));
            double actE = battN->getBatteryLevelAbs();
            sumVar += pow(actE - meanEnergy, 2.0);
        }
        varEnergy = sumVar / (((double) numberNodes) - 1.0);

        recordScalar("FINALENERGYSUM", sumEnergy);
        recordScalar("FINALENERGYMEAN", meanEnergy);
        recordScalar("FINALENERGYVAR", varEnergy);
        recordScalar("FINALENERGYMAX", maxEnergy);
        recordScalar("LIFETIME", simTime());

        if (countCoverage > 0) {
            recordScalar("COVERAGE_AVERAGE", sumCoverageTot / countCoverage);
            recordScalar("COVERAGE_RATIO_AVERAGE", sumCoverageRatioTot / countCoverage);
            recordScalar("COVERAGE_RATIO_MAX_AVERAGE", sumCoverageRatioMaxTot / countCoverage);

            for (int i = 0; i < N_PERCENTAGE_COVERAGE; i++) {
                char scalarName[64];
                double actPercentage = (((double) (i + 1.0)) / ((double) (N_PERCENTAGE_COVERAGE))) * 100.0;
                double ris = 0;

                /******************** INSTANT FAILURE ********************************/
                if (firstCoveragePassPercent[i] == false) {
                    ris = 0;
                }
                else if (endCoveragePassPercent[i] == 0) {
                    ris = simTime().dbl();
                }
                else {
                    ris = endCoveragePassPercent[i].dbl();
                }
                snprintf(scalarName, sizeof(scalarName), "LIFETIME_COVERAGE_%.0lf", actPercentage);
                recordScalar(scalarName, ris);
                snprintf(scalarName, sizeof(scalarName), "LIFETIME_COVERAGE_RATIOMAX_%.0lf", actPercentage);
                recordScalar(scalarName, ris/areaMaxToCoverage);

                /******************** 5 seconds FAILURE ********************************/
                if (firstCoveragePassPercent[i] == false) {
                    ris = 0;
                }
                else if (endCoveragePassPercent2[i] == 0) {
                    ris = simTime().dbl();
                }
                else {
                    ris = endCoveragePassPercent2[i].dbl();
                }
                snprintf(scalarName, sizeof(scalarName), "LIFETIME_COVERAGE_5_%.0lf", actPercentage);
                recordScalar(scalarName, ris);
                snprintf(scalarName, sizeof(scalarName), "LIFETIME_COVERAGE_5_RATIOMAX_%.0lf", actPercentage);
                recordScalar(scalarName, ris/areaMaxToCoverage);

                /******************** 10 seconds FAILURE ********************************/
                if (firstCoveragePassPercent[i] == false) {
                    ris = 0;
                }
                else if (endCoveragePassPercent3[i] == 0) {
                    ris = simTime().dbl();
                }
                else {
                    ris = endCoveragePassPercent3[i].dbl();
                }
                snprintf(scalarName, sizeof(scalarName), "LIFETIME_COVERAGE_10_%.0lf", actPercentage);
                recordScalar(scalarName, ris);
                snprintf(scalarName, sizeof(scalarName), "LIFETIME_COVERAGE_10_RATIOMAX_%.0lf", actPercentage);
                recordScalar(scalarName, ris/areaMaxToCoverage);
            }
        }
    }

    recordScalar("FINAL_BATTERY", sb->getBatteryLevelAbs());

    recordScalar("FAILED_ATTEMPT_COUNT", failedAttemptCount);
    recordScalar("FAILED_ATTEMPT_FREQ", ((double)failedAttemptCount)/simTime().dbl());

    recordScalar("TOTAL_ATTEMPT_COUNT", totalAttemptCount);
    recordScalar("TOTAL_ATTEMPT_FREQ", ((double)totalAttemptCount)/simTime().dbl());
}


void UDPRechargeBasic::handleMessageWhenUp(cMessage *msg) {
    if (msg == autoMsgRecharge) {

        if(!(goToCharge->isScheduled())) {

            if (sb->getState() == power::SimpleBattery::CHARGING){
                bool stopCharging = checkDischarge();

                inRechargingTime += checkRechargeTimer;

                if (stopCharging) {

                    //stop the node
                    if (rechargeIsolation) {
                        neigh.clear();
                        filter_neigh.clear();
                    }
                    //mob->clearVirtualSprings();
                    mob->clearVirtualSpringsAndsetPosition(rebornPos);

                    sb->setState(power::SimpleBattery::DISCHARGING);

                    lastSawInRecharge = simTime();

                    //STATS
                    timeOfRechargeVector.record(simTime() - startRecharge);

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

                    /*double distReborn = rebornPos.distance(mob->getCurrentPosition());
                    double secs = distReborn / mob->par("maxspeed").doubleValue();

                    sendRechargeMessage();
                    scheduleAt(simTime() + secs, goToCharge);*/
                }
            }
        }

        scheduleAt(simTime() + checkRechargeTimer, autoMsgRecharge);
    }
    else if (msg == goToCharge) {

        totalAttemptCount++;

        if (checkRechargingStationFree()) {

            rechargeLostAccess = 0;
            startRecharge = simTime();

            energyAtRecharge = sb->getBatteryLevelAbs();

            sb->setState(power::SimpleBattery::CHARGING);

            //mob->clearVirtualSprings();
            mob->clearVirtualSpringsAndsetPosition(rebornPos);

        }
        else {
            rechargeLostAccess++;
            failedAttemptCount++;

            lastSawInRecharge = simTime();

            sb->setDoubleSwapPenality();

            double distReborn = rebornPos.distance(mob->getCurrentPosition());
            double secs = distReborn / mob->par("maxspeed").doubleValue();

            if (secs > (autoMsgRecharge->getArrivalTime() - simTime()).dbl()) {
                secs = (autoMsgRecharge->getArrivalTime() - simTime()).dbl() - 0.001;
            }

            looseRechargingChance = true;
            scheduleAt(simTime() + secs, backAfterLoose);
            //backAfterLoose->isScheduled()

            //mob->clearVirtualSprings();
            mob->clearVirtualSpringsAndsetPosition(rebornPos);
        }

        //stop the node
        if (rechargeIsolation) {
            neigh.clear();
            filter_neigh.clear();
        }
        //mob->clearVirtualSprings();
        //mob->clearVirtualSpringsAndsetPosition(rebornPos);

    }
    else if (msg == backAfterLoose) {
        looseRechargingChance = false;
    }
    else if (msg == posMessage) {
        sendPositionMessage();
        scheduleAt(simTime() + positionMessageTimer, posMessage);
    }
    else if (msg == stat5sec) {
        make5secStats();
        scheduleAt(simTime() + 5, msg);
    }
    else if (msg == stat1sec) {
        updateNeighbourhood();
        updateVirtualForces();

        make1secStats();

        if (myAppAddr == 0) {
            checkAlive();
        }

        if (useEnergyToShare) {
            if(timeToUpdateEnergy <= simTime()){
                energyToShare = sb->getBatteryLevelAbs();

                timeToUpdateEnergy = simTime() + delayTimeToUpdateEnergy;
            }
        }

        scheduleAt(simTime() + 1, msg);
    }
    else {
        UDPBasicApp::handleMessageWhenUp(msg);
    }
}
void UDPRechargeBasic::checkAlive(void) {
    int numberNodes = this->getParentModule()->getVectorSize();

    for (int i = 0; i < numberNodes; i++) {
        power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", i)->getSubmodule("battery"));

        if (battN->getBatteryLevelAbs() <= 0) {
            endSimulation();
        }
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
    if ((sb->getState() == power::SimpleBattery::DISCHARGING) || (!rechargeIsolation)) {
        if (dynamic_cast<ApplicationPacketRecharge *> (pk)) {
            ApplicationPacketRecharge *aPkt = check_and_cast<ApplicationPacketRecharge *> (pk);
            if (myAddr != aPkt->getAddr()) {

                cObject *c = pk->getControlInfo();
                UDPDataIndicationExt *di = check_and_cast<UDPDataIndicationExt *>(c);

                //EV_DEBUG << "Received recharge packet " << aPkt->getName() << " with " << di->getFullName() << endl;

                if (aPkt->getGoingToRecharge()) {
                    if (neigh.count(aPkt->getAppAddr()) != 0) {
                        neigh.erase(aPkt->getAppAddr());
                    }
                    if (saveNeighboursMsgs) {
                        lastSawInRecharge = simTime();
                    }
                }
                else {

                    std::stringstream ss;

                    if (neigh.count(aPkt->getAppAddr()) == 0) {
                        nodeInfo_t newInfo;
                        newInfo.addr = aPkt->getAddr();
                        newInfo.appAddr = aPkt->getAppAddr();
                        newInfo.hasPosInfo = false;
                        newInfo.hasEnergyInfo = false;

                        neigh[aPkt->getAppAddr()] = newInfo;

                        ss << "[" << myAppAddr << "] - ADDING NEIGHBOUR: ";
                    }
                    else {
                        ss << "[" << myAppAddr << "] - UPDATING NEIGHBOUR: ";
                    }

                    nodeInfo_t *node = &(neigh[aPkt->getAppAddr()]);

                    if (!sendDifferentMessages) {
                        node->hasPosInfo = true;
                        node->pos = aPkt->getPos();
                    }
                    node->rcvPow = di->getPow();
                    node->rcvSnr = di->getSnr();

                    node->timestamp = simTime();

                    if (saveNeighboursMsgs) {

                        node->batteryLevelAbs = aPkt->getBatteryLevelAbs();
                        node->batteryLevelPerc = aPkt->getBatteryLevelPerc();
                        node->coveragePercentage = aPkt->getCoveragePercentage();
                        node->leftLifetime = aPkt->getLeftLifetime();
                        node->nodeDegree = aPkt->getNodeDegree();
                        node->inRechargeT = aPkt->getInRecharge();
                        node->gameTheoryC = aPkt->getGameTheoryC();
                        node->gameTheoryPC = aPkt->getGameTheoryPC();
                        node->recharging = aPkt->getRecharging();
                        node->hasEnergyInfo = true;
                    }

                    ss << node->appAddr << ". NEIG.SIZE: " << neigh.size();
                    //fprintf(stderr, "%s\n", ss.str().c_str());fflush(stderr);
                    EV << ss.str() << endl;
                }

                getFilteredNeigh(filter_neigh);

                if (!sendDifferentMessages) {

                    if ((sb->getState() == power::SimpleBattery::DISCHARGING) && (!looseRechargingChance)) {
                        updateVirtualForces();
                    }
                }

                EV << "[" << myAppAddr << "] - NEW PACKET arrived. Neigh size: " << neigh.size() << endl;
            }
        }
        else {
            ApplicationPacketPosition *aPkt = check_and_cast<ApplicationPacketPosition *> (pk);
            if (myAddr != aPkt->getAddr()) {

                cObject *c = pk->getControlInfo();
                UDPDataIndicationExt *di = check_and_cast<UDPDataIndicationExt *>(c);

                if (neigh.count(aPkt->getAppAddr()) == 0) {
                    nodeInfo_t newInfo;
                    newInfo.addr = aPkt->getAddr();
                    newInfo.appAddr = aPkt->getAppAddr();
                    newInfo.hasPosInfo = false;
                    newInfo.hasEnergyInfo = false;

                    neigh[aPkt->getAppAddr()] = newInfo;
                }

                nodeInfo_t *node = &(neigh[aPkt->getAppAddr()]);

                node->pos = aPkt->getPos();
                node->rcvPow = di->getPow();
                node->rcvSnr = di->getSnr();
                node->timestamp = simTime();
                node->hasPosInfo = true;

                getFilteredNeigh(filter_neigh);

                if ((sb->getState() == power::SimpleBattery::DISCHARGING) && (!looseRechargingChance)) {
                    updateVirtualForces();
                }
            }
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
    if (useEnergyToShare) {
        payload->setBatteryLevelAbs(getEnergyToShare());
    }
    else {
        payload->setBatteryLevelAbs(sb->getBatteryLevelAbs());
    }
    payload->setBatteryLevelPerc(sb->getBatteryLevelPerc());
    payload->setCoveragePercentage(0);
    payload->setLeftLifetime(sb->getBatteryLevelAbs() / sb->getDischargingFactor(checkRechargeTimer));
    payload->setNodeDegree(calculateNodeDegree());
    payload->setInRecharge(inRechargingTime);
    payload->setGoingToRechargeTime(calculateRechargeTime());
    payload->setGameTheoryC(getGameTheoryC());
    payload->setGameTheoryPC(getGameTheoryPC());
    payload->setRecharging(sb->getState() == power::SimpleBattery::CHARGING);

    return payload;
}

ApplicationPacketPosition *UDPRechargeBasic::generatePktPosToSend(const char *name) {
    ApplicationPacketPosition *payload = new ApplicationPacketPosition(name);
    //payload->setByteLength(par("messageLength").longValue());
    payload->setByteLength(4 * sizeof(double));
    payload->setSequenceNumber(numSent);

    payload->setPos(mob->getCurrentPosition());
    payload->setAddr(myAddr);
    payload->setAppAddr(myAppAddr);

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

void UDPRechargeBasic::sendPositionMessage(void) {
    std::ostringstream str;
    str << packetName << "-POSITION-" << numSent;

    //ApplicationPacketRecharge *payload = generatePktToSend(str.str().c_str(), true);
    ApplicationPacketPosition *payload = generatePktPosToSend(str.str().c_str());

    L3Address destAddr = chooseDestAddr();
    emit(sentPkSignal, payload);
    socket.sendTo(payload, destAddr, destPort);
    numSent++;
}


void UDPRechargeBasic::sendPacket()
{
    if ((sb->getState() == power::SimpleBattery::DISCHARGING) || (!rechargeIsolation)) {
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
    double cw = 0.45;
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

    if (activateVirtualForceMovements) {

        for (auto it = neigh.begin(); it != neigh.end(); it++) {
            nodeInfo_t *act = &(it->second);
            if ((!act->recharging) && (act->hasPosInfo)) {
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
        }

        // add the force towards the center rebornPos
        if (rebornPos.distance(myPos) > 10) {
            Coord uVec = rebornPos - myPos;
            //Coord uVec = myPos - rebornPos;
            uVec.normalize();
            mob->addVirtualSpring(uVec, rebornPos.distance(myPos), -3);
        }
    }
}

void UDPRechargeBasic::updateNeighbourhood(void) {
    bool removed, removedAtLeastOne;
    removedAtLeastOne = false;
    do {
        removed = false;
        for (auto it = neigh.begin(); it != neigh.end(); it++) {
            nodeInfo_t *act = &(it->second);

            if ((simTime() - act->timestamp) > (2.0 * par("sendInterval").doubleValue())) {
                std::stringstream ss;
                ss << "[" << myAppAddr << "] - UPDATE NEIGHBOURHOOD. Removing: " << it->second.appAddr << " ";
                ss << "- NOW: " << simTime() << " - timestamp: " << act->timestamp;
                //fprintf(stderr, "%s\n", ss.str().c_str());fflush(stderr);
                EV << ss.str() << endl;

                neigh.erase(it);
                removed = true;
                removedAtLeastOne = true;
                break;
            }
        }
    } while(removed);

    if (removedAtLeastOne) {
        getFilteredNeigh(filter_neigh);
    }
}

void UDPRechargeBasic::getFilteredNeigh(std::map<int, nodeInfo_t> &filteredNeigh){
    std::list<VirtualSpringMobility::NodeBasicInfo> nodesToFilter;
    std::list<VirtualSpringMobility::NodeBasicInfo> nodeFiltered;

    VirtualSpringMobility::NodeBasicInfo recharginginfo;
    recharginginfo.id = -1;

    filteredNeigh.clear();

    for (auto it = neigh.begin(); it != neigh.end(); it++) {
        if (it->second.recharging) {
            recharginginfo.id = it->first;
        }
        else {
            VirtualSpringMobility::NodeBasicInfo newinfo;
            newinfo.id = it->first;
            newinfo.position = it->second.pos;
            nodesToFilter.push_back(newinfo);
        }
    }
    mob->filterNodeListAcuteAngleTest(nodesToFilter, nodeFiltered);
    if (recharginginfo.id >= 0) {
        nodeFiltered.push_back(recharginginfo);
    }

    for (auto it = neigh.begin(); it != neigh.end(); it++) {
        nodeInfo_t *act = &(it->second);

        for (auto it2 = nodeFiltered.begin(); it2 != nodeFiltered.end(); it2++) {
            if ((act->appAddr == it2->id) && (act->hasEnergyInfo)) {
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
    return 0.0;
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

    if (myAppAddr == 0) {
        int nnodesActive = 0;
        int nnodesRecharging = 0;
        double sumEnergy = 0.0;
        double maxEnergy = 0.0;
        double minEnergy = 10000000000.0;
        double sumVar = 0.0;
        double meanEnergy, varEnergy;

        int numberNodes = this->getParentModule()->getVectorSize();

        for (int i = 0; i < numberNodes; i++) {
            power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", i)->getSubmodule("battery"));
            double actE = battN->getBatteryLevelAbs();

            if (battN->isCharging()) {
                nnodesRecharging++;
            }
            else {
                nnodesActive++;
            }

            sumEnergy += actE;
            if (maxEnergy < actE) maxEnergy = actE;
            if (minEnergy > actE) minEnergy = actE;
        }
        meanEnergy = sumEnergy / ((double)numberNodes);
        energyVectorAllMean.record(meanEnergy);
        energyVectorAllMin.record(minEnergy);
        energyVectorAllMax.record(maxEnergy);
        energyVectorAllMaxMinDiff.record(maxEnergy-minEnergy);

        for (int i = 0; i < numberNodes; i++) {
            power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", i)->getSubmodule("battery"));
            double actE = battN->getBatteryLevelAbs();
            sumVar += pow(actE - meanEnergy, 2.0);
        }
        varEnergy = sumVar / (((double) numberNodes) - 1.0);
        energyVectorAllVar.record(varEnergy);

        activeNodesVector.record(nnodesActive);
        rechargingNodesVector.record(nnodesRecharging);

        if (makeCoverageLog){
            double m2covered = getFullCoverage(makeCoverageMap);
            double fullArea = mob->getConstraintAreaMax().x * mob->getConstraintAreaMax().y;
            double fullAreaRatio = m2covered/fullArea;
            double fullAreaMaxRatio = m2covered/areaMaxToCoverage;

            if (fullAreaMaxRatio > 1) fullAreaMaxRatio = 1;

            totalCoverageVector.record(m2covered);
            totalCoverageRatioVector.record(fullAreaRatio);
            totalCoverageRatioMaxVector.record(fullAreaMaxRatio);

            sumCoverageTot += m2covered;
            sumCoverageRatioTot += fullAreaRatio;
            sumCoverageRatioMaxTot += fullAreaMaxRatio;
            countCoverage += 1.0;

            for (int i = 0; i < N_PERCENTAGE_COVERAGE; i++) {
                double actRatio = ((double) (i + 1.0)) / ((double) (N_PERCENTAGE_COVERAGE));
                if ((fullAreaMaxRatio >= actRatio) && (!firstCoveragePassPercent[i])) {
                    firstCoveragePassPercent[i] = true;
                }

                if ((fullAreaMaxRatio < actRatio) && (firstCoveragePassPercent[i])) {
                    numberCoveragePassPercent[i]++;

                    if (endCoveragePassPercent[i] == 0) {
                        endCoveragePassPercent[i] = simTime();
                    }

                    if ((numberCoveragePassPercent[i] > 1) && (endCoveragePassPercent2[i] == 0)) {
                        endCoveragePassPercent2[i] = simTime();
                    }
                    if ((numberCoveragePassPercent[i] > 2) && (endCoveragePassPercent3[i] == 0)) {
                        endCoveragePassPercent3[i] = simTime();
                    }
                }

                if (fullAreaMaxRatio >= actRatio) {
                    numberCoveragePassPercent[i] = 0;
                }
            }
        }
    }


    degreeVector.record((double) filter_neigh.size());
    fulldegreeVector.record((double) neigh.size());
    failedAttemptVector.record(failedAttemptCount);
    if (sb->isCharging()) {
        dischargeProbVector.record(calculateDischargeProb());
    }
    else {
        responseVector.record(calculateRechargeProb());
    }
    hypotheticalDischargeProbVector.record(calculateDischargeProb());
    hypotheticalResponseVector.record(calculateRechargeProb());

    energyVector.record(sb->getBatteryLevelAbs());

}

void UDPRechargeBasic::make1secStats(void) {
}

double UDPRechargeBasic::getFullCoverage(bool makeMap) {

    // create the groups
    int activeNodes = 0;
    double actArea = 0.0;
    std::vector< std::vector<int> > matrixVal;

    //Grow rows by matrixsideSize
    matrixVal.resize(mob->getConstraintAreaMax().x);
    for(int i = 0 ; i < (int)matrixVal.size() ; ++i) {
        //Grow Columns by matrixsideSize
        matrixVal[i].resize(mob->getConstraintAreaMax().y);
        for(int j = 0 ; j < (int)matrixVal[i].size() ; ++j) {      //modify matrix
            matrixVal[i][j] = 0;
        }
    }

    for (int i = 0; i < numberNodesInSimulation; i++) {
        UDPRechargeBasic *rb = check_and_cast<UDPRechargeBasic *>(this->getParentModule()->getParentModule()->getSubmodule("host", i)->getSubmodule("udpApp",0));
        VirtualSpringMobility *mobN = check_and_cast<VirtualSpringMobility *>(this->getParentModule()->getParentModule()->getSubmodule("host", i)->getSubmodule("mobility"));
        power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", i)->getSubmodule("battery"));

        if ((!(battN->isCharging())) && (!(rb->getLooseRechargingChance()))) {
            activeNodes++;
            for(int i = 0 ; i < (int)matrixVal.size() ; ++i) {
                for(int j = 0 ; j < (int)matrixVal[i].size() ; ++j) {      //modify matrix
                    Coord point = Coord(i,j);
                    if(point.distance(mobN->getCurrentPosition()) <= sensorRadious) {
                        if (matrixVal[i][j] == 0) {
                            actArea += 1.0;
                        }
                        matrixVal[i][j]++;
                    }
                }
            }
        }
    }

    if (makeMap) {
        FILE * fmap;
        int n;
        char buff[256];

        snprintf(buff, sizeof(buff), "%s.cmap%03d.map", coverageMapFilename.c_str(), coverageMapIdx);
        coverageMapIdx++;

        fmap = fopen(buff, "w");
        if (fmap) {
            for(int i = 0 ; i < (int)matrixVal.size() ; ++i) {
                for(int j = 0 ; j < (int)matrixVal[i].size() ; ++j) {
                    n = snprintf(buff, sizeof(buff), "%d ", matrixVal[i][j]);
                    fwrite(buff, sizeof(char), n, fmap);
                }
                n = snprintf(buff, sizeof(buff), "\n");
                fwrite(buff, sizeof(char), n, fmap);
            }
            fclose (fmap);
        }
    }
    //printMatrix(matrixVal);

    //double actArea = 0.0;
    //for(int i = 0 ; i < (int)matrixVal.size() ; ++i) {
    //    for(int j = 0 ; j < (int)matrixVal[i].size() ; ++j) {      //modify matrix
    //        if (matrixVal[i][j] == true) {
    //            actArea += 1.0;
    //        }
    //    }
    //}
    //double maxArea = ((double) numberNodesInSimulation) * ((sensorRadious*sensorRadious) * (3.0 / 2.0) * sqrt(3.0));
    //double maxArea = ((double) numberNodesInSimulation) * ((sensorRadious*sensorRadious) * 2.598076211);
    //double maxArea = ((double) (numberNodesInSimulation - chargingStationNumber)) * ((sensorRadious*sensorRadious) * 2.598076211);
    //double maxArea = ((double) (activeNodes)) * ((sensorRadious*sensorRadious) * 2.598076211);

    //double ratio = actArea / maxArea;

    //if (ratio > 1.0) ratio = 1.0;

    //return ratio;
    return actArea;
}

} /* namespace inet */


