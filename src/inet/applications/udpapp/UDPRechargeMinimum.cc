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

#include <UDPRechargeMinimum.h>

namespace inet {

Define_Module(UDPRechargeMinimum)


UDPRechargeMinimum::~UDPRechargeMinimum() {
}

void UDPRechargeMinimum::initialize(int stage)
{
    UDPRechargeBasic::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        useDischargeProbability = par("useDischargeProbability").boolValue();
        useOptimalRechargeTime = par("useOptimalRechargeTime").boolValue();
        chargeSlots = par("chargeSlots");

        std::string minimKnowledgeType_str = par("minimKnowledgeType").stdstringValue();
        if (minimKnowledgeType_str.compare("LOCAL_KNOWLEDGE") == 0) {
            minimKnowledgeType = LOCAL_KNOWLEDGE;
        }
        else if (minimKnowledgeType_str.compare("GLOBAL_KNOWLEDGE") == 0) {
            minimKnowledgeType = GLOBAL_KNOWLEDGE;
        }
        else if (minimKnowledgeType_str.compare("PERSONAL_KNOWLEDGE") == 0) {
            minimKnowledgeType = PERSONAL_KNOWLEDGE;
        }
        else {
            error("Wrong \"minimKnowledgeType\" parameter");
        }

        countRechargeSlot = 0;
    }
    else if (stage == INITSTAGE_LAST) {

    }
}

void UDPRechargeMinimum::handleMessageWhenUp(cMessage *msg) {
    if (msg == autoMsgRecharge) {
        if (sb->isCharging()) {
            countRechargeSlot++;
        }
        else {
            countRechargeSlot = 0;
        }
    }

    UDPRechargeBasic::handleMessageWhenUp(msg);
}

double UDPRechargeMinimum::calculateRechargeProb(void){
    if (sb->isCharging()) {
        return 0.0;
    }
    else {
        double ris = 1;
        double emin;
        switch (minimKnowledgeType) {
            case PERSONAL_KNOWLEDGE:
            default:
                ris = 1.0 - sb->getBatteryLevelPercInitial();
                break;

            case LOCAL_KNOWLEDGE:
            case GLOBAL_KNOWLEDGE:
                emin = getEmin(false, minimKnowledgeType);

                if (emin == sb->getBatteryLevelAbs()) {
                    ris = 1.0;
                }
                else {
                    ris = 0.0;
                }
                break;
        }
        return ris;
    }
}

double UDPRechargeMinimum::calculateDischargeProb(void){
    if (useOptimalRechargeTime) {
        double ris = 0.0;
        double timeInCharge;
        double estimatedTimeInRecharging = calculateEstimatedTimeInRecharging();

        if (sb->isCharging()) {
            timeInCharge = (simTime() - startRecharge).dbl();
        }
        else {
            timeInCharge = (simTime() - lastSawInRecharge).dbl();

        }

        estimatedTimeInRecharging = estimatedTimeInRecharging * checkRechargeTimer;

        //fprintf(stderr, "timeInCharge: %lf and estimatedTimeInRecharging = %lf\n", timeInCharge, estimatedTimeInRecharging); fflush(stderr);

        //ris = timeInCharge / estimatedTimeInRecharging;

        //if (ris > 1) ris = 1;
        //if (ris < 0) ris = 0;

        if (timeInCharge >= estimatedTimeInRecharging) {
            ris = 1;
        }
        else {
            ris = 0;
        }


        return ris;
    }
    else {
        if (useDischargeProbability) {
            //return dischargeProbability;
            return (1.0 / ((double) chargeSlots));
        }
        else {
            return calculateStaticDischargeProbability();
        }
    }
}

double UDPRechargeMinimum::calculateStaticDischargeProbability(void) {
    if (!sb->isCharging()) {
        return 0.0;
    }
    else {
        if (countRechargeSlot >= chargeSlots) {
            return 1.0;
        }
        else {
            return 0.0;
        }
    }
}

double UDPRechargeMinimum::getEmax(bool activeOnly, MinimumKnowledge_Type scope) {
    int numberNodes = this->getParentModule()->getVectorSize();
    //double max = 0;
    double max = sb->getBatteryLevelAbs();
    if (scope == GLOBAL_KNOWLEDGE) {
        for (int j = 0; j < numberNodes; j++) {
            power::SimpleBattery *hostjsb = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", j)->getSubmodule("battery"));

            if (activeOnly && hostjsb->isCharging()) continue;

            if (hostjsb->getBatteryLevelAbs() > max){
                max = hostjsb->getBatteryLevelAbs();
            }
        }
    }
    else if (scope == LOCAL_KNOWLEDGE){
        if (sb->isCharging()){
            for (auto it = neighBackupWhenRecharging.begin(); it != neighBackupWhenRecharging.end(); it++) {
                nodeInfo_t *act = &(it->second);
                double actBatt = act->batteryLevelAbs;
                if (actBatt > max) {
                    max = actBatt;
                }
            }

        }
        else {
            for (auto it = neigh.begin(); it != neigh.end(); it++) {
                nodeInfo_t *act = &(it->second);
                double actBatt = act->batteryLevelAbs;
                if (actBatt > max) {
                    max = actBatt;
                }
            }
        }
    }
    else if (scope == PERSONAL_KNOWLEDGE){
        return sb->getBatteryLevelAbs();
    }
    else {
        error("Wrong knowledge scope");
    }

    return max;
}

double UDPRechargeMinimum::getEmin(bool activeOnly, MinimumKnowledge_Type scope) {
    int numberNodes = this->getParentModule()->getVectorSize();
    //double min = 1000000000;
    double min = sb->getBatteryLevelAbs();
    if (scope == GLOBAL_KNOWLEDGE) {
        for (int j = 0; j < numberNodes; j++) {
            power::SimpleBattery *hostjsb = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", j)->getSubmodule("battery"));

            if (activeOnly && hostjsb->isCharging()) continue;

            if (hostjsb->getBatteryLevelAbs() < min){
                min = hostjsb->getBatteryLevelAbs();
            }
        }
    }
    else if (scope == LOCAL_KNOWLEDGE){
        if (sb->isCharging()){
            for (auto it = neighBackupWhenRecharging.begin(); it != neighBackupWhenRecharging.end(); it++) {
                nodeInfo_t *act = &(it->second);
                double actBatt = act->batteryLevelAbs;
                if (actBatt < min) {
                    min = actBatt;
                }
            }
        }
        else {
            for (auto it = neigh.begin(); it != neigh.end(); it++) {
                nodeInfo_t *act = &(it->second);
                double actBatt = act->batteryLevelAbs;
                if (actBatt < min) {
                    min = actBatt;
                }
            }
        }
    }
    else if (scope == PERSONAL_KNOWLEDGE){
        return sb->getBatteryLevelAbs();
    }
    else {
        error("Wrong knowledge scope");
    }

    return min;
}

double UDPRechargeMinimum::getEavg(bool activeOnly, MinimumKnowledge_Type scope) {
    int numberNodes = this->getParentModule()->getVectorSize();
    double sum = 0;
    double nn;
    if (scope == GLOBAL_KNOWLEDGE) {
        for (int j = 0; j < numberNodes; j++) {
            power::SimpleBattery *hostjsb = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", j)->getSubmodule("battery"));

            if (activeOnly && hostjsb->isCharging()) continue;

            sum += hostjsb->getBatteryLevelAbs();
        }
        nn = numberNodes;
    }
    else if (scope == LOCAL_KNOWLEDGE){
        sum = sb->getBatteryLevelAbs();
        if ((sb->isCharging()) && (rechargeIsolation)){
            for (auto it = neighBackupWhenRecharging.begin(); it != neighBackupWhenRecharging.end(); it++) {
                nodeInfo_t *act = &(it->second);
                sum += act->batteryLevelAbs;
            }
            nn = neighBackupWhenRecharging.size() + 1;
        }
        else {
            for (auto it = filter_neigh.begin(); it != filter_neigh.end(); it++) {
                nodeInfo_t *act = &(it->second);
                sum += act->batteryLevelAbs;
            }
            nn = filter_neigh.size() + 1;
        }
    }
    else if (scope == PERSONAL_KNOWLEDGE){
        return sb->getBatteryLevelAbs();
    }
    else {
        error("Wrong knowledge scope");
    }

    return (sum / nn);
}

double UDPRechargeMinimum::calculateEstimatedTimeInRecharging(void) {
    double estimatedTimeInRecharging, energyToUse, timeCalcNum, timeCalcDen1, timeCalcDen2, gPLUSt;
    int numberNodes = this->getParentModule()->getVectorSize();

    energyToUse = getEavg(false, minimKnowledgeType);

    gPLUSt = sb->getSwapLoose() + sb->getSwapLoose();

    timeCalcNum = energyToUse - gPLUSt;
    timeCalcDen1 = sb->getDischargingFactor(checkRechargeTimer) * ((double)(numberNodes - 1.0));
    timeCalcDen2 = ((double)(numberNodes - 1.0)) * gPLUSt;

    estimatedTimeInRecharging = timeCalcNum / (timeCalcDen1 + timeCalcDen2);

    return estimatedTimeInRecharging;
}

} /* namespace inet */
