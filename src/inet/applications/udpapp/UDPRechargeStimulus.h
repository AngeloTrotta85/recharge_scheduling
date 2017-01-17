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

#ifndef INET_APPLICATIONS_UDPAPP_UDPRECHARGESTIMULUS_H_
#define INET_APPLICATIONS_UDPAPP_UDPRECHARGESTIMULUS_H_

#include "inet/common/INETDefs.h"

#include <vector>
#include <map>
#include <list>
#include <iomanip>      // std::setprecision

#include "inet/applications/udpapp/UDPRechargeBasic.h"

namespace inet {

class INET_API UDPRechargeStimulus : public UDPRechargeBasic {

public:
    typedef enum {
        MIN_VAL,
        MAX_VAL,
        AVG_VAL
    } RechargeLength_Type;

protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void processPacket(cPacket *msg);

    virtual double calculateSwapPenalitiesEstimationCount(double estimatedSteps);
    virtual double calculateChargeDiff (double myChoice);
    virtual double calculateRechargeTime(bool log);

    virtual double calculateRechargeStimuliEnergyFactor(void);
    virtual double calculateRechargeStimuliTimeFactor(void);

    virtual double calculateRechargeStimuli(void);
    virtual double calculateRechargeThreshold(void);

public:
    virtual ~UDPRechargeStimulus();

    virtual double calculateRechargeProb(void);
    virtual double calculateDischargeProb(void);

protected:
    double stimulusExponent;
    int numRechargeSlotsStimulusZeroNeigh;
    RechargeLength_Type rlt;
    bool stationANDnodeKNOWN;
    double chargeTimeOthersNodeFactor;
    bool makeLowEnergyFactorCurves;
    bool useProbabilisticDischarge;
    bool useQuadraticProbabilisticDischarge;

    bool firstRecharge;
    simtime_t lastRechargeTimestamp;
    double slotsInCharge;
    int countRechargeSlot;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_UDPAPP_UDPRECHARGESTIMULUS_H_ */
