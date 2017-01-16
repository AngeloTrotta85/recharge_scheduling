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
        scheduleAt(simTime() + checkRechargeTimer, autoMsgRecharge);
    }
    else {
        UDPRechargeBasic::handleMessageWhenUp(msg);
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


} /* namespace inet */
