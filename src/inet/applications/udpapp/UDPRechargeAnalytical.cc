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

#include <UDPRechargeAnalytical.h>

#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(UDPRechargeAnalytical)

UDPRechargeAnalytical::~UDPRechargeAnalytical() {

}

void UDPRechargeAnalytical::initialize(int stage)
{
    UDPRechargeBasic::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        std::string schedulingType = par("schedulingType").stdstringValue();
        //ANALYTICAL, ROUNDROBIN, STIMULUS
        if (schedulingType.compare("ANALYTICAL") == 0) {
            st = ANALYTICAL;
        }
        else if (schedulingType.compare("ROUNDROBIN") == 0) {
            st = ROUNDROBIN;
        }
        else {
            error("Wrong \"schedulingType\" parameter");
        }

        roundrobinRechargeSize = par("roundrobinRechargeSize");

        printAnalticalLog = par("printAnalticalLog").boolValue();
        snprintf(logFile, sizeof(logFile), "%s", par("analticalLogFile").stringValue());
        remove(logFile);

    }
    else if (stage == INITSTAGE_LAST) {
        myAddr = L3AddressResolver().resolve(this->getParentModule()->getFullPath().c_str());
        //myAppAddr = this->getParentModule()->getIndex();
        EV << "[" << myAppAddr << "] My address is: " << myAddr << std::endl;

        this->getParentModule()->getDisplayString().setTagArg("t", 0, myAddr.str().c_str());

        initCentralizedRecharge();

        if (myAppAddr != 0) {   // NODE 0 MAKES EVERYTHING
            if (autoMsgRecharge->isScheduled()) {
                cancelEvent(autoMsgRecharge);
            }
        }
    }
}

void UDPRechargeAnalytical::handleMessageWhenUp(cMessage *msg) {
    if (msg == autoMsgRecharge) {
        checkCentralizedRecharge();
        scheduleAt(simTime() + checkRechargeTimer, autoMsgRecharge);
    }
    else {
        UDPRechargeBasic::handleMessageWhenUp(msg);
    }
}

// comparison, energy.
//bool UDPBasicRecharge::compare_energy (const nodeAlgo_t& first, const nodeAlgo_t& second) {
bool compare_energy (const inet::UDPRechargeAnalytical::nodeAlgo_t& first, const inet::UDPRechargeAnalytical::nodeAlgo_t& second) {
    //power::SimpleBattery *batt1 = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", first.addr)->getSubmodule("battery"));
    //power::SimpleBattery *batt2 = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", second.addr)->getSubmodule("battery"));

    //return (batt1->getBatteryLevelAbs() < batt2->getBatteryLevelAbs());
    //EV << std::scientific << std::setprecision(20) << "First " << first.energy << ":" << first.isCharging << " - Second " << second.energy << ":" << second.isCharging << endl;

    if (fabs(first.energy - second.energy) < EPSILON) {
        if (first.isCharging) {
            return true;
        }
        return false;
    }
    else {
        return first.energy < second.energy;
    }
}

bool compare_charge (const inet::UDPRechargeAnalytical::nodeAlgo_t& first, const inet::UDPRechargeAnalytical::nodeAlgo_t& second) {
    if (first.isCharging) {
        return true;
    }
    else if (second.isCharging) {
        return false;
    }
    else {
        return first.energy < second.energy;
    }
}

void UDPRechargeAnalytical::initCentralizedRecharge(void) {
    // create the groups
    int numberNodes = this->getParentModule()->getVectorSize();
    int numG = numberNodes / chargingStationNumber;

    if ((numberNodes % chargingStationNumber) > 0) numG++;

    int actG = 0;
    for (int i = 0; i < numberNodes; i++) {


        if (actG == 0) {
            groupInfo_t newG;

            newG.chargingAppAddr = -1;
            newG.swapNumber = 0;
            //newG.nodeList.push_front(newNodeInfo);

            groupList.push_front(newG);
        }
        //else {
        //    groupList.front().nodeList.push_front(i);
        //}

        nodeAlgo_t newNodeInfo;
        newNodeInfo.addr = i;
        newNodeInfo.executedRecharge = 0;
        newNodeInfo.assignedRecharge = 0;
        newNodeInfo.isCharging = false;
        newNodeInfo.energy = 0;

        groupList.front().nodeList.push_front(newNodeInfo);

        actG++;
        if (actG >= numG) {
            actG = 0;
        }
    }

    decideRechargeSceduling();

    checkCentralizedRecharge();
}

bool UDPRechargeAnalytical::decideRechargeSceduling(void) {
    bool ris = true;
    for (auto it = groupList.begin(); it != groupList.end(); it++) {
        groupInfo_t *actGI = &(*it);
        bool groupRis = true;

        if (st == ANALYTICAL) {
            groupRis = decideRechargeScedulingGroup(actGI);
        }
        else if (st == ROUNDROBIN){
            groupRis = decideRechargeScedulingGroupRR(actGI);
        }

        ris = ris && groupRis;
    }

    return ris;
}

void UDPRechargeAnalytical::updateBatteryVals(std::list<nodeAlgo_t> *list) {
    for (auto itn = list->begin(); itn != list->end(); itn++){
        nodeAlgo_t *actNO = &(*itn);
        power::SimpleBattery *batt = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", actNO->addr)->getSubmodule("battery"));
        actNO->energy = batt->getBatteryLevelAbs();
    }
}

void UDPRechargeAnalytical::checkCentralizedRecharge(void) {
    for (auto it = groupList.begin(); it != groupList.end(); it++) {
        groupInfo_t *actGI = &(*it);
        updateBatteryVals(&(actGI->nodeList));
    }

    for (auto it = groupList.begin(); it != groupList.end(); it++) {
        groupInfo_t *actGI = &(*it);

        checkCentralizedRechargeGroup(actGI);

        //checkAliveGroup(actGI);
    }
    printChargingInfo();

    if (printAnalticalLog) {
        FILE *f = fopen(logFile, "a");
        if (f) {
            std::stringstream ss;
            printChargingInfo(ss, "BATTERY INFO -> ");
            fwrite(ss.str().c_str(), 1 , ss.str().length(), f);
            fclose(f);
        }
        else {
            fprintf(stderr, "Error opening file: %s \n", logFile); fflush(stderr);
            perror("Error writing on file");
            error("Error writing on file\n");
        }
    }
}


bool UDPRechargeAnalytical::decideRechargeScedulingGroupRR(groupInfo_t *actGI) {
    bool ris = true;

    for (auto itn = actGI->nodeList.begin(); itn != actGI->nodeList.end(); itn++){
        nodeAlgo_t *actNO = &(*itn);

        actNO->executedRecharge = 0;
        actNO->assignedRecharge = roundrobinRechargeSize;
    }

    return ris;
}

bool UDPRechargeAnalytical::checkScheduleFeasibilityGroup(groupInfo_t *actGI) {
    //check the feasibility of the schedule
    bool feasible = true;
    int rechargeSteps = 0;

    for (auto itn = actGI->nodeList.begin(); itn != actGI->nodeList.end(); itn++){
        nodeAlgo_t *actNO = &(*itn);
        rechargeSteps += actNO->assignedRecharge;
    }

    int beforeME = 0;
    for (auto itn = actGI->nodeList.begin(); itn != actGI->nodeList.end(); itn++){
        nodeAlgo_t *actNO = &(*itn);

        //int othersRechargeSteps = rechargeSteps - actNO->assignedRecharge;

        //int possibleSteps = (actNO->energy - 1 + (actNO->assignedRecharge * sb->getChargingFactor())) / sb->getDischargingFactor();
        //int possibleSteps = ((actNO->energy - 1 - (2.0 * sb->getSwapLoose())) / sb->getDischargingFactor(checkRechargeTimer))
        //        + actNO->assignedRecharge;


        // check if I can reach my recharge slots
        if (feasible) {
            double energyBeforeRecharge = actNO->energy -1 - (beforeME * sb->getDischargingFactor(checkRechargeTimer)) - sb->getSwapLoose();
            if (energyBeforeRecharge <= 0) {
                feasible = false;
                EV << "I cannot reach the recharge steps. EnergyBeforeRecharge: " << energyBeforeRecharge << endl;
            }
            else {
                EV << "I can reach the recharge steps. EnergyBeforeRecharge: " << energyBeforeRecharge << endl;
            }
        }

        if (feasible) {
            int nextME = rechargeSteps - beforeME - actNO->assignedRecharge;
            double energyAfterRecharge = actNO->energy - 1
                    - (beforeME * sb->getDischargingFactor(checkRechargeTimer))
                    - (2 * sb->getSwapLoose())
                    + (actNO->assignedRecharge * sb->getChargingFactor(checkRechargeTimer));
            if ((energyAfterRecharge - (nextME * sb->getDischargingFactor(checkRechargeTimer))) <= 0 ) {
                feasible = false;
                EV << "I cannot reach the final steps. EnergyAtTheEnd: " << (energyAfterRecharge - (nextME * sb->getDischargingFactor(checkRechargeTimer))) << endl;
            }
            else {
                EV << "I can reach the final steps. EnergyAtTheEnd: " << (energyAfterRecharge - (nextME * sb->getDischargingFactor(checkRechargeTimer))) << endl;
            }
        }

        //int requestSteps = rechargeSteps;

        //EV << "RequestSteps: " << rechargeSteps << " - PossibleSteps: " << possibleSteps << endl;

        //if ((rechargeSteps < ((int)actGI->nodeList.size())) || (possibleSteps < rechargeSteps)) {
        //    EV << "NOT FEASIBLE!!!" << endl;
        //    feasible = false;
        //   break;
        //}

        beforeME += actNO->assignedRecharge;
    }

    return feasible;
}

bool UDPRechargeAnalytical::decideRechargeScedulingGroup(groupInfo_t *actGI) {
    double maxE;
    bool ris = true;

    getNodeWithMaxEnergy(actGI, maxE);

    //int numSteps = (maxE - 1) / sb->getDischargingFactor(checkRechargeTimer);
    int numSteps = (maxE - (2.0 * sb->getSwapLoose())) / sb->getDischargingFactor(checkRechargeTimer);
    int numChargeSlots = numSteps / (actGI->nodeList.size() - 1);
    //int plusSteps = ((int) maxE) % ((int)sb->getDischargingFactor());
    int plusSteps = numSteps - (numChargeSlots * (actGI->nodeList.size() - 1));


    updateBatteryVals(&(actGI->nodeList));
    printChargingInfo("BEFORE SORT -> ");
    actGI->nodeList.sort(compare_energy);
    printChargingInfo("AFTER SORT -> ");

    for (auto itn = actGI->nodeList.begin(); itn != actGI->nodeList.end(); itn++){
        nodeAlgo_t *actNO = &(*itn);

        actNO->executedRecharge = 0;
        actNO->assignedRecharge = numChargeSlots;
        if (plusSteps > 0) {
            actNO->assignedRecharge++;
            plusSteps--;
        }
    }

    printChargingInfo("BEFORE CHECKING FEASIBILITY -> ");

    if (!checkScheduleFeasibilityGroup(actGI)) {

        for (auto itn = actGI->nodeList.begin(); itn != actGI->nodeList.end(); itn++){
            nodeAlgo_t *actNO = &(*itn);

            actNO->executedRecharge = 0;
            actNO->assignedRecharge = 1;
        }
        actGI->nodeList.sort(compare_charge);

        printChargingInfo("BEFORE SECOND CHECKING FEASIBILITY -> ");

        if (!checkScheduleFeasibilityGroup(actGI)) {
            decideRechargeScedulingGroupLast(actGI);
            ris = false;
        }
    }

    printChargingInfo("AFTER Scheduling decision -> ");

    return ris;
}

void UDPRechargeAnalytical::checkCentralizedRechargeGroup(groupInfo_t *actGI) {
    //updateBatteryVals(&(actGI->nodeList));

    for (auto itn = actGI->nodeList.begin(); itn != actGI->nodeList.end(); itn++){
        nodeAlgo_t *actNO = &(*itn);

        if (actGI->chargingAppAddr < 0) {
            // no-body is charging (this is the first time)
            actNO->isCharging = true;
            actGI->chargingAppAddr = actNO->addr;

            putNodeInCharging(actNO->addr);

            EV << "FIRST SCHEDULER STEP" << endl;

            // PUT THE OTHERS IN DISCHARGE
            itn++;
            for (; itn != actGI->nodeList.end(); itn++){
                nodeAlgo_t *actNO3 = &(*itn);
                putNodeInDischarging(actNO3->addr);
            }

            break;
        }
        else if (actNO->isCharging) {

            // recharging slot executed
            actNO->executedRecharge++;

            if (actNO->executedRecharge == actNO->assignedRecharge) {
                // recharge fase finished... swap the charging uav with the next one
                //actNO->isCharging = false;

                itn++;
                if (itn != actGI->nodeList.end()) {
                    nodeAlgo_t *actNO2 = &(*itn);

                    if (actNO2->assignedRecharge > 0) {
                        actNO->isCharging = false;
                        putNodeInDischarging(actNO->addr);

                        actNO2->isCharging = true;
                        putNodeInCharging(actNO2->addr);

                        actGI->chargingAppAddr = actNO2->addr;
                        actGI->swapNumber++;
                    }
                }
                else {
                    // I'm the last one, so start again
                    //decideRechargeScedulingGroup(actGI);
                    if (st == ANALYTICAL) {
                        decideRechargeScedulingGroup(actGI);

                        if (!actGI->nodeList.begin()->isCharging) {

                            EV << "THE FIRST ONE IS NOT IN CHARGING !!!!! WHY???" << endl;
                            printChargingInfo("BEFORE SORT -> ");
                            actGI->nodeList.sort(compare_charge);
                            printChargingInfo("AFTER SORT -> ");

                            /*if (nextPossible) {
                                EV << "THE FIRST ONE IS NOT IN CHARGING !!!!! WHY???" << endl;

                                actGI->chargingAppAddr = -1;
                                for (itn = actGI->nodeList.begin(); itn != actGI->nodeList.end(); itn++){
                                    nodeAlgo_t *actNO3 = &(*itn);
                                    actNO3->isCharging = false;
                                    putNodeInDischarging(actNO3->addr);
                                }

                                checkCentralizedRechargeGroup(actGI);
                            }*/
                        }

                        //actGI->chargingAppAddr = -1;
                        //actNO->isCharging = false;
                        //putNodeInDischarging(actNO->addr);

                        //checkCentralizedRechargeGroup(actGI);
                    }
                    else if (st == ROUNDROBIN){
                        actGI->chargingAppAddr = -1;
                        actNO->isCharging = false;
                        putNodeInDischarging(actNO->addr);
                        decideRechargeScedulingGroupRR(actGI);
                        checkCentralizedRechargeGroup(actGI);
                    }
                }

                break;
            }
            //else {
                //nothing to do
            //}
        }
    }
}

void UDPRechargeAnalytical::decideRechargeScedulingGroupLast(groupInfo_t *actGI) {
    for (auto itn = actGI->nodeList.begin(); itn != actGI->nodeList.end(); itn++){
        nodeAlgo_t *actNO = &(*itn);
        actNO->assignedRecharge = 0;
    }

    updateBatteryVals(&(actGI->nodeList));
    actGI->nodeList.sort(compare_energy);
    actGI->nodeList.sort(compare_charge);

    // schedule to make sure "change when dying"
    int sumDischargingSteps = 0;
    for (auto itn = actGI->nodeList.begin(); itn != actGI->nodeList.end(); itn++){
        nodeAlgo_t *actNO = &(*itn);
        auto itn2 = itn;
        itn2++;

        if (itn2 != actGI->nodeList.end()) {
            nodeAlgo_t *nextNO = &(*itn2);

            // to calculate the possible steps check the next node's energy (minus 1), remove all the previous discharging steps
            // and finally divide on the discharge factor

            //EV << "Next energy: " << nextNO->energy << "; steps done: " << sumDischargingSteps;
            actNO->assignedRecharge = (nextNO->energy - 1 - (sumDischargingSteps * sb->getDischargingFactor(checkRechargeTimer))) / sb->getDischargingFactor(checkRechargeTimer);
            //EV << "; assigned: " << actNO->assignedRecharge << endl;
        }
        else {
            actNO->assignedRecharge = (actNO->energy - 1 - (sumDischargingSteps * sb->getDischargingFactor(checkRechargeTimer))) / sb->getDischargingFactor(checkRechargeTimer);
        }

        if (actNO->assignedRecharge < 0) {
            actNO->assignedRecharge = 0;
        }

        sumDischargingSteps += actNO->assignedRecharge;

        if (actNO->assignedRecharge == 0) break;
    }
}

void UDPRechargeAnalytical::putNodeInCharging(int addr) {
    // setting the node in charging
    VirtualSpringMobility *mobN = check_and_cast<VirtualSpringMobility *>(this->getParentModule()->getParentModule()->getSubmodule("host", addr)->getSubmodule("mobility"));
    power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", addr)->getSubmodule("battery"));
    UDPRechargeAnalytical *nodeN = check_and_cast<UDPRechargeAnalytical *>(this->getParentModule()->getParentModule()->getSubmodule("host", addr)->getSubmodule("udpApp", 0));

    battN->setState(power::SimpleBattery::CHARGING);
    mobN->clearVirtualSpringsAndsetPosition(rebornPos);
    nodeN->neigh.clear();
}

void UDPRechargeAnalytical::putNodeInDischarging(int addr) {
    // setting the node in charging
    VirtualSpringMobility *mobN = check_and_cast<VirtualSpringMobility *>(this->getParentModule()->getParentModule()->getSubmodule("host", addr)->getSubmodule("mobility"));
    power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", addr)->getSubmodule("battery"));

    battN->setState(power::SimpleBattery::DISCHARGING);
    mobN->clearVirtualSpringsAndsetPosition(rebornPos);
}

int UDPRechargeAnalytical::getNodeWithMaxEnergy(groupInfo_t *gi, double &battVal) {
    int ris = -1;
    double maxE = -1;

    for (auto it = gi->nodeList.begin(); it != gi->nodeList.end(); it++) {
        nodeAlgo_t *actN = &(*it);
        power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", actN->addr)->getSubmodule("battery"));

        if (battN->getBatteryLevelAbs() > maxE) {
            ris = actN->addr;
            maxE = battN->getBatteryLevelAbs();
        }
    }

    battVal = maxE;
    return ris;
}

int UDPRechargeAnalytical::getNodeWithMinEnergy(groupInfo_t *gi, double &battVal) {
    int ris = -1;
    double minE = std::numeric_limits<double>::max();

    for (auto it = gi->nodeList.begin(); it != gi->nodeList.end(); it++) {
        nodeAlgo_t *actN = &(*it);
        power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", actN->addr)->getSubmodule("battery"));

        if (battN->getBatteryLevelAbs() < minE) {
            ris = actN->addr;
            minE = battN->getBatteryLevelAbs();
        }
    }

    battVal = minE;
    return ris;

}

void UDPRechargeAnalytical::printChargingInfo(std::ostream &ss, const char *str) {
    for (auto it = groupList.begin(); it != groupList.end(); it++) {
        groupInfo_t *actGI = &(*it);
        ss << ((int) simTime().dbl()) << " - ";
        ss << str;
        for (auto itn = actGI->nodeList.begin(); itn != actGI->nodeList.end(); itn++){
            nodeAlgo_t *actNO = &(*itn);
            ss << "[" << actNO->addr << "]" << actNO->energy << "|" << actNO->executedRecharge << "/" << actNO->assignedRecharge << "|" << actNO->isCharging << "  ";
        }
        ss << endl;
    }
}

void UDPRechargeAnalytical::printChargingInfo(const char *str) {
    printChargingInfo(EV, str);
}

void UDPRechargeAnalytical::printChargingInfo(void) {
    printChargingInfo("BATTERY INFO -> ");
}

} /* namespace inet */
