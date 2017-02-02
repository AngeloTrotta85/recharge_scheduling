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

#include <UDPRechargeProbabilistic.h>

namespace inet {

Define_Module(UDPRechargeProbabilistic)


UDPRechargeProbabilistic::~UDPRechargeProbabilistic() {
}

void UDPRechargeProbabilistic::initialize(int stage)
{
    UDPRechargeBasic::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        useDischargeProbability = par("useDischargeProbability").boolValue();
        chargeSlots = par("chargeSlots");

        std::string probKnowledgeType_str = par("probKnowledgeType").stdstringValue();
        if (probKnowledgeType_str.compare("LOCAL_KNOWLEDGE") == 0) {
            probKnowledgeType = LOCAL_KNOWLEDGE;
        }
        else if (probKnowledgeType_str.compare("GLOBAL_KNOWLEDGE") == 0) {
            probKnowledgeType = GLOBAL_KNOWLEDGE;
        }
        else if (probKnowledgeType_str.compare("PERSONAL_KNOWLEDGE") == 0) {
            probKnowledgeType = PERSONAL_KNOWLEDGE;
        }
        else {
            error("Wrong \"probKnowledgeType\" parameter");
        }

        countRechargeSlot = 0;
    }
    else if (stage == INITSTAGE_LAST) {

    }
}

void UDPRechargeProbabilistic::handleMessageWhenUp(cMessage *msg) {
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

double UDPRechargeProbabilistic::calculateRechargeProb(void){
    if (sb->isCharging()) {
        return 0.0;
    }
    else {
        double ris = 1;
        double emax, emin;
        switch (probKnowledgeType) {
            case PERSONAL_KNOWLEDGE:
            default:
                ris = 1.0 - sb->getBatteryLevelPercInitial();
                break;

            case LOCAL_KNOWLEDGE:
            case GLOBAL_KNOWLEDGE:
                emax = getEmax(false, probKnowledgeType);
                emin = getEmin(false, probKnowledgeType);
                if (emax <= emin) {
                    ris = 1.0 - sb->getBatteryLevelPercInitial();
                }
                else {
                    ris = (emax - sb->getBatteryLevelAbs()) / (emax - emin);
                }
                break;
        }
        return ris;
    }
}

double UDPRechargeProbabilistic::calculateDischargeProb(void){
    if (useDischargeProbability) {
        //return dischargeProbability;
        return (1.0 / ((double) chargeSlots));
    }
    else {
        return calculateStaticDischargeProbability();
    }
}

double UDPRechargeProbabilistic::calculateStaticDischargeProbability(void) {
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

double UDPRechargeProbabilistic::getEmax(bool activeOnly, ProbabilisticKnowledge_Type scope) {
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

double UDPRechargeProbabilistic::getEmin(bool activeOnly, ProbabilisticKnowledge_Type scope) {
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

} /* namespace inet */
