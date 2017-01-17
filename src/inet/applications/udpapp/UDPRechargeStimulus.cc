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

#include <UDPRechargeStimulus.h>

namespace inet {

Define_Module(UDPRechargeStimulus)


UDPRechargeStimulus::~UDPRechargeStimulus() {
}

void UDPRechargeStimulus::initialize(int stage)
{
    UDPRechargeBasic::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        stimulusExponent = par("stimulusExponent");
        numRechargeSlotsStimulusZeroNeigh = par("numRechargeSlotsStimulusZeroNeigh");
        stationANDnodeKNOWN = par("stationANDnodeKNOWN").boolValue();
        chargeTimeOthersNodeFactor = par("chargeTimeOthersNodeFactor");
        makeLowEnergyFactorCurves = par("makeLowEnergyFactorCurves").boolValue();
        useProbabilisticDischarge = par("useProbabilisticDischarge").boolValue();
        useQuadraticProbabilisticDischarge = par("useQuadraticProbabilisticDischarge").boolValue();

        if (useQuadraticProbabilisticDischarge && useProbabilisticDischarge) {
            error("useQuadraticProbabilisticDischarge and useProbabilisticDischarge are both true");
        }

        std::string chargeLengthType = par("chargeLengthType").stdstringValue();
        if (chargeLengthType.compare("MIN") == 0) {
            rlt = MIN_VAL;
        }
        else if (chargeLengthType.compare("MAX") == 0) {
            rlt = MAX_VAL;
        }
        else if (chargeLengthType.compare("AVG") == 0) {
            rlt = AVG_VAL;
        }
        else {
            error("Wrong \"chargeLengthType\" parameter");
        }


        firstRecharge = true;
        lastRechargeTimestamp = simTime();
        inRechargingTime = 0;
        slotsInCharge = 0;
        countRechargeSlot = 0;
    }
    else if (stage == INITSTAGE_LAST) {

    }
}

void UDPRechargeStimulus::handleMessageWhenUp(cMessage *msg) {
    if (msg == autoMsgRecharge) {
        if (sb->isCharging()){
            lastRechargeTimestamp = simTime();

            countRechargeSlot++;
        }
        else {
            countRechargeSlot = 0;
        }
    }

    UDPRechargeBasic::handleMessageWhenUp(msg);

    if (msg == goToCharge) {
        firstRecharge = false;

        if (sb->isCharging()){
            slotsInCharge = calculateRechargeTime(true) / checkRechargeTimer;
        }
    }
}

void UDPRechargeStimulus::processPacket(cPacket *pk) {
    if (firstRecharge) {
        ApplicationPacketRecharge *aPkt = check_and_cast<ApplicationPacketRecharge *> (pk);
        if ((myAddr != aPkt->getAddr()) && aPkt->getGoingToRecharge() ) {
            firstRecharge = false;
        }
    }

    UDPRechargeBasic::processPacket(pk);
}

double UDPRechargeStimulus::calculateRechargeProb(void){

    if (!sb->isCharging()) {
        return 0.0;
    }
    else {
        double s = calculateRechargeStimuli();
        double t = calculateRechargeThreshold();

        return (pow(s, stimulusExponent) / (pow(s, stimulusExponent) + pow(t, stimulusExponent)));
    }
}


double UDPRechargeStimulus::calculateSwapPenalitiesEstimationCount(double estimatedSteps) {
    double ris = 0;
    int nSteps = estimatedSteps * ((((double) this->getParentModule()->getVectorSize()) / ((double) chargingStationNumber)) - 1.0);

    for (int i = 1; i <= nSteps; i++) {
        ris += ((double) i) / ((double) nSteps);
    }

    return ris;
}



double UDPRechargeStimulus::calculateChargeDiff (double myChoice) {
    double ris = myChoice;

    if (neigh.size() > 0) {
        std::map<int, nodeInfo_t> filteredNeigh;
        getFilteredNeigh(filteredNeigh);

        double maxC = -1;
        for (auto it = filteredNeigh.begin(); it != filteredNeigh.end(); it++) {
            nodeInfo_t *act = &(it->second);

            if (act->inRechargeT > maxC) {
                maxC = act->inRechargeT;
            }
        }

        if ((maxC > 0) && (maxC > (inRechargingTime + myChoice))) {
            ris = maxC - inRechargingTime;
        }
    }

    return ris;
}

double UDPRechargeStimulus::calculateRechargeTime(bool log) {

    double recTime = 0;
    std::stringstream ss;


    // default charge until full charge
    //double tt = ((sb->getFullCapacity() - sb->getBatteryLevelAbs()) / sb->getChargingFactor(checkRechargeTimer)) * checkRechargeTimer;
    double tt = numRechargeSlotsStimulusZeroNeigh * checkRechargeTimer;

    if (log) ss << "RECHARGETIME STIMULUS: Default charge time: " << tt << " - Neigh size: " << neigh.size() << endl;

    if (neigh.size() > 0) {

        std::map<int, nodeInfo_t> filteredNeigh;
        getFilteredNeigh(filteredNeigh);

        double sumE = sb->getBatteryLevelAbs();
        double maxE = sb->getBatteryLevelAbs();
        double minE = sb->getBatteryLevelAbs();
        if (log) ss << "RECHARGETIME STIMULUS: my battery: " << sb->getBatteryLevelAbs() << endl;
        for (auto it = filteredNeigh.begin(); it != filteredNeigh.end(); it++) {
            //for (auto it = neigh.begin(); it != neigh.end(); it++) {
            nodeInfo_t *act = &(it->second);

            sumE += act->batteryLevelAbs;
            if (act->batteryLevelAbs > maxE)
                maxE = act->batteryLevelAbs;
            if (act->batteryLevelAbs < minE)
                minE = act->batteryLevelAbs;

            if (log) ss << "RECHARGETIME STIMULUS: others battery: " << act->batteryLevelAbs << endl;

        }

        //double averageE = sumE / (((double) neigh.size()) + 1.0);
        //double averageE = sumE / (((double) nodeFiltered.size()) + 1.0);
        double averageE = sumE / (((double) filteredNeigh.size()) + 1.0);

        if (log) ss << "RECHARGETIME STIMULUS: Average Energy: " << averageE << ", Max Energy: " << maxE
                //if (log) ss << "RECHARGETIME STIMULUS: Average Energy: " << averageE << ", Max Energy: " << maxE
                << " - Discharging Factor: " << sb->getDischargingFactor(checkRechargeTimer)
                << " - SwapLoose Factor: " << sb->getSwapLoose()
                << " - NodeFiltered size: " << filteredNeigh.size()
                << " - Neigh size: " << neigh.size()
                //<< " - stationANDnodeKNOWN: " << stationANDnodeKNOWN
                //<< " - reinforcementRechargeTime: " << reinforcementRechargeTime
                << " - checkRechargeTimer: " << checkRechargeTimer
                << endl;

        //double numSteps = (maxE - (2.0 * sb->getSwapLoose())) / sb->getDischargingFactor(checkRechargeTimer);
        double valToUse = 1;
        switch (rlt) {
        case MIN_VAL:
            valToUse = minE;
            break;

        case MAX_VAL:
            valToUse = maxE;
            break;

        case AVG_VAL:
            valToUse = averageE;
            break;

        default:
            error("Wring rlt value");
            break;
        }
        double tmpnumSteps = (valToUse - (2.0 * sb->getSwapLoose())) / sb->getDischargingFactor(checkRechargeTimer);
        double swapPenalitiesEstimation = calculateSwapPenalitiesEstimationCount(tmpnumSteps/checkRechargeTimer) * (2.0 * sb->getSwapLoose());
        double numSteps = (valToUse - (2.0 * sb->getSwapLoose()) - swapPenalitiesEstimation) / sb->getDischargingFactor(checkRechargeTimer);

        if (log) ss << "RECHARGETIME STIMULUS: swapPenalitiesEstimation: " << swapPenalitiesEstimation <<
                ", tmpnumSteps: " << tmpnumSteps <<
                ", numSteps: " << numSteps << endl;

        //int actualNeigh = neigh.size();
        //if (actualNeigh > 7) actualNeigh = 7;
        //double numChargeSlots = numSteps / ((double) neigh.size());
        //double numChargeSlots = numSteps / ((double) actualNeigh);
        //double numChargeSlots = numSteps / ((double) nodeFiltered.size());
        double numChargeSlots;
        if (stationANDnodeKNOWN) {
            int numberNodes = this->getParentModule()->getVectorSize();
            numChargeSlots = numSteps / ((((double) numberNodes) / ((double) chargingStationNumber)) - 1.0);

            if (log) ss << "RECHARGETIME STIMULUS: numSteps: " << numSteps <<
                    ", numberNodes: " << numberNodes <<
                    ", chargingStationNumber: " << chargingStationNumber <<
                    ", n-1: " << ((((double) numberNodes) / ((double) chargingStationNumber)) - 1.0) <<
                    ", numChargeSlots: " << numChargeSlots <<
                    endl;
        }
        else {
            numChargeSlots = numSteps / ((double) filteredNeigh.size() + 1.0);
        }
        //double numChargeSlots = numSteps / ((double) filteredNeigh.size() + 1.0);
        tt = numChargeSlots * checkRechargeTimer;

        //tt = (averageE / ((sb->getDischargingFactor(checkRechargeTimer)) * ((double) neigh.size()))) * checkRechargeTimer;
        //tt = (maxE / ((sb->getDischargingFactor(checkRechargeTimer)) * ((double) neigh.size()))) * checkRechargeTimer;


        if (chargeTimeOthersNodeFactor > 0) {
            double diffOthers = calculateChargeDiff(tt);
            if (diffOthers > 0) {
                if (log) ss << "RECHARGETIME STIMULUS: my tt: " << tt << endl;
                tt = (chargeTimeOthersNodeFactor * diffOthers) + ((1.0 - chargeTimeOthersNodeFactor) * tt);
                if (log) ss << "RECHARGETIME STIMULUS: diffOthers: " << diffOthers
                        << " chargeTimeOthersNodeFactor: " << chargeTimeOthersNodeFactor
                        << endl;
            }
        }
    }

    recTime = tt;


    if (log) {
        ss << "RECHARGETIME Final decision charge time: " << recTime << endl;

        fprintf(stderr, "%s", ss.str().c_str());
    }

    return recTime;
}

double UDPRechargeStimulus::calculateRechargeStimuliTimeFactor(void) {
    //double averageE = 0;
    double sumE = sb->getBatteryLevelAbs();
    double maxE = sb->getBatteryLevelAbs();

    if (sb->getState() != power::SimpleBattery::DISCHARGING){
        return 0;
    }

    std::map<int, nodeInfo_t> filteredNeigh;
    getFilteredNeigh(filteredNeigh);

    for (auto it = filteredNeigh.begin(); it != filteredNeigh.end(); it++) {
    //for (auto it = neigh.begin(); it != neigh.end(); it++) {
        nodeInfo_t *act = &(it->second);

        sumE += act->batteryLevelAbs;
        if (act->batteryLevelAbs > maxE)
            maxE = act->batteryLevelAbs;
    }
    //averageE = sumE / ((double) (neigh.size() + 1));
    //averageE = sumE / ((double) (filteredNeigh.size() + 1));

    double rechargeEstimation = ((sb->getFullCapacity() - sb->getBatteryLevelAbs()) / sb->getChargingFactor(checkRechargeTimer)) * checkRechargeTimer;
    if (filteredNeigh.size() > 0) {
    //if (neigh.size() > 0) {
        //rechargeEstimation = (averageE / ((sb->getDischargingFactor(checkRechargeTimer)) * ((double) neigh.size()))) * checkRechargeTimer;
        //rechargeEstimation = (maxE / ((sb->getDischargingFactor(checkRechargeTimer)) * ((double) neigh.size()))) * checkRechargeTimer;
        rechargeEstimation = (maxE / ((sb->getDischargingFactor(checkRechargeTimer)) * ((double) filteredNeigh.size()))) * checkRechargeTimer;
    }

    rechargeEstimation = calculateRechargeTime(false);

    //double timeFactor = (simTime() - lastRechargeTimestamp).dbl() / (rechargeEstimation * timeFactorMultiplier);
    //double timeFactor = (simTime() - lastRechargeTimestamp).dbl() / (rechargeEstimation * ((double) filteredNeigh.size()));
    //timeFactor = timeFactor / timeFactorMultiplier;

    double timeFactor = (simTime() - lastRechargeTimestamp).dbl() / (rechargeEstimation * ((double) filteredNeigh.size()));
    if (stationANDnodeKNOWN) {
        int numberNodes = this->getParentModule()->getVectorSize();
        timeFactor = (simTime() - lastRechargeTimestamp).dbl() / (rechargeEstimation * (((double) numberNodes) / ((double) chargingStationNumber)));
    }
    if (timeFactor > 1) timeFactor = 1;

    return timeFactor;

}

double UDPRechargeStimulus::calculateRechargeStimuliEnergyFactor(void) {
    double maxE = sb->getBatteryLevelAbs();
    double minE = maxE;

    for (auto it = neigh.begin(); it != neigh.end(); it++) {
        nodeInfo_t *act = &(it->second);
        double actBatt = act->batteryLevelAbs;

        if (actBatt > maxE) {
            maxE = actBatt;
        }
        if (actBatt < minE) {
            minE = actBatt;
        }

        //fprintf(stderr, "BATT: %f, MAX: %f, MIN: %f\n", actBatt, maxE, minE);fflush(stderr);
    }
    if (maxE == minE) {
        return 1;
    }
    if (sb->getState() != power::SimpleBattery::DISCHARGING){
        return 1;
    }
    double ris = (sb->getBatteryLevelAbs() - minE) / (maxE - minE);

    //fprintf(stderr, "[%d] - Energy Factor. MAX: %f; MIN: %f, MY: %f, Ris: %f\n", myAppAddr, maxE, minE, sb->getBatteryLevelAbs(), ris);fflush(stderr);

    return ris;

}

double UDPRechargeStimulus::calculateRechargeStimuli(void) {
    if (sb->isCharging()){
        return 0;
    }
    if (firstRecharge) {
        return dblrand();
    }
    else {
        if (makeLowEnergyFactorCurves) {
            return pow(calculateRechargeStimuliTimeFactor(), (1.0 / (1.0 - calculateRechargeStimuliEnergyFactor())));
        }
        else {
            return pow(calculateRechargeStimuliTimeFactor(), calculateRechargeStimuliEnergyFactor());
        }
    }
}


double UDPRechargeStimulus::calculateRechargeThreshold(void) {
    int myDegree = calculateNodeDegree();
    /*int maxDegree = myDegree;
    for (auto it = neigh.begin(); it != neigh.end(); it++) {
        nodeInfo_t *act = &(it->second);
        if (maxDegree < act->nodeDegree) {
            maxDegree = act->nodeDegree;
        }
    }
    if (maxDegree > 0) {
        return (((double)myDegree) / ((double)maxDegree));
    }
    else {
        return 0;
    }*/
    double ris = 0;

    ris = ((double) myDegree) / 6.0;
    if (ris > 1) ris = 1;

    return ris;
}

double UDPRechargeStimulus::calculateDischargeProb(void){
    if (sb->isCharging()) {
        return 0.0;
    }
    else {
        if (useProbabilisticDischarge){
            return (1.0 - slotsInCharge);
        }
        else if (useQuadraticProbabilisticDischarge) {
            if (countRechargeSlot >= slotsInCharge) {
                return 1.0;
            }
            else {
                return pow(countRechargeSlot / slotsInCharge, 2.0);
            }
        }
        else {
            if (countRechargeSlot >= slotsInCharge) {
                return 1.0;
            }
            else {
                return 0.0;
            }
        }
        //return (1.0 - timeInCharge);
    }
}

} /* namespace inet */
