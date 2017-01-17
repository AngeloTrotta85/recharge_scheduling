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
        dischargeProbability = par("dischargeProbability");
        chargeSlots = par("chargeSlots");

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
        return (1.0 - sb->getBatteryLevelPercInitial());
    }
}

double UDPRechargeProbabilistic::calculateDischargeProb(void){
    if (useDischargeProbability) {
        return dischargeProbability;
    }
    else {
        return calculateDischargeProbability();
    }
}

double UDPRechargeProbabilistic::calculateDischargeProbability(void) {
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

} /* namespace inet */
