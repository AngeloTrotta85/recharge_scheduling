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

#include <UDPRechargeGameTheory.h>

namespace inet {

Define_Module(UDPRechargeGameTheory)


UDPRechargeGameTheory::~UDPRechargeGameTheory() {
}

void UDPRechargeGameTheory::initialize(int stage)
{
    UDPRechargeBasic::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        variableC = par("variableC").boolValue();
        variableP = par("variableP").boolValue();
        linearIncreaseFactor = par("linearIncreaseFactor");
        constDischargeProb = par("constDischargeProb");
        exponential_dischargeProb_decay = par("exponential_dischargeProb_decay");
        temp_factorProbDischarge = par("temp_factorProbDischarge");

        std::string gameTheoryKnowledgeType_str = par("gameTheoryKnowledgeType").stdstringValue();
        if (gameTheoryKnowledgeType_str.compare("LOCAL_KNOWLEDGE") == 0) {
            gameTheoryKnowledgeType = LOCAL_KNOWLEDGE;
        }
        else if (gameTheoryKnowledgeType_str.compare("GLOBAL_KNOWLEDGE") == 0) {
            gameTheoryKnowledgeType = GLOBAL_KNOWLEDGE;
        }
        else {
            error("Wrong \"gameTheoryKnowledgeType\" parameter");
        }

        std::string dischargeProbEnergyToUseType = par("dischargeProbEnergyToUse").stdstringValue();
        if (dischargeProbEnergyToUseType.compare("ENERGYMIN") == 0) {
            dischargeProbEnergyToUse = ENERGYMIN;
        }
        else if (dischargeProbEnergyToUseType.compare("ENERGYMAX") == 0) {
            dischargeProbEnergyToUse = ENERGYMAX;
        }
        else if (dischargeProbEnergyToUseType.compare("ENERGYAVG") == 0) {
            dischargeProbEnergyToUse = ENERGYAVG;
        }
        else {
            error("Wrong \"dischargeProbEnergyToUse\" parameter");
        }

        std::string constType = par("varConstantType").stdstringValue();
        //"SIGMOID", "LINEAR1", "LINEAR2"
        if (constType.compare("LINEARINCREASE") == 0) {
            constant_type = LINEARINCREASE;
        }
        else if (constType.compare("LINEARINCREASECONSISTENT") == 0) {
            constant_type = LINEARINCREASECONSISTENT;
        }
        else {
            error("Wrong \"varConstantType\" parameter");
        }
    }
    else if (stage == INITSTAGE_LAST) {

    }
}

void UDPRechargeGameTheory::handleMessageWhenUp(cMessage *msg) {
    if (msg == autoMsgRecharge) {

    }

    UDPRechargeBasic::handleMessageWhenUp(msg);
}



double UDPRechargeGameTheory::calculateRechargeProb(void){
    if (sb->isCharging()) {
        return 0.0;
    }
    else {
        int numberNodes = this->getParentModule()->getVectorSize();
        double ris, s;

        if (variableC) {

            long double dischargeP = -1;
            long double produttoria = 1.0;
            double nmeno1SquareRoot, unomenoCi;

            if (variableP){
                if (gameTheoryKnowledgeType == GLOBAL_KNOWLEDGE) {
                    for (int j = 0; j < numberNodes; j++) {
                        UDPRechargeGameTheory *hostj = check_and_cast<UDPRechargeGameTheory *>(this->getParentModule()->getParentModule()->getSubmodule("host", j)->getSubmodule("udpApp", 0));
                        power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", j)->getSubmodule("battery"));

                        if (battN->isCharging()) {
                            dischargeP = hostj->calculateDischargeProb();
                        }
                    }
                }
                else if (gameTheoryKnowledgeType == LOCAL_KNOWLEDGE) {
                    // use mine
                    dischargeP = calculateDischargeProb();
                }
                else {
                    error("Wrong knowledge scope");
                }

            }

            unomenoCi = 1.0 - getGameTheoryC();
            if (unomenoCi <= 0){
                s = 0.0;
            }
            else {
                if (gameTheoryKnowledgeType == GLOBAL_KNOWLEDGE) {
                    for (int j = 0; j < numberNodes; j++) {
                        UDPRechargeGameTheory *hostj = check_and_cast<UDPRechargeGameTheory *>(this->getParentModule()->getParentModule()->getSubmodule("host", j)->getSubmodule("udpApp", 0));
                        double hostC = hostj->getGameTheoryC();
                        long double ppp = (1.0 - hostC) / unomenoCi;
                        produttoria = produttoria * ppp;

                        //fprintf(stderr, "%Lf ", ppp);

                    }
                }
                else if (gameTheoryKnowledgeType == LOCAL_KNOWLEDGE) {
                    for (auto it = neigh.begin(); it != neigh.end(); it++) {
                        nodeInfo_t *act = &(it->second);

                        double hostC = act->gameTheoryC;
                        long double ppp = (1.0 - hostC) / unomenoCi;
                        produttoria = produttoria * ppp;
                    }
                }
                else {
                    error("Wrong knowledge scope");
                }

                if (dischargeP > 0) {
                    produttoria = produttoria * (unomenoCi / dischargeP);
                }
                else {
                    produttoria = produttoria * unomenoCi;
                }

                if (gameTheoryKnowledgeType == GLOBAL_KNOWLEDGE) {
                    nmeno1SquareRoot = powl(produttoria, 1.0 / (((long double) numberNodes) - 1.0));
                }
                else if (gameTheoryKnowledgeType == LOCAL_KNOWLEDGE){
                    nmeno1SquareRoot = powl(produttoria, 1.0 / ((long double) neigh.size()));
                }
                else {
                    error("Wrong knowledge scope");
                }

                s = nmeno1SquareRoot;

                if (s > 1) s = 1;
                if (s < 0) s = 0;
            }

            ris = 1.0 - s;
        }
        else {
            double c = (getGamma()+getTheta()) / (getAlpha() + getBeta());
            if (variableP){
                c = c / constDischargeProb;
            }
            s = pow(c, 1.0 / (((double) numberNodes) - 1.0));

            ris = 1.0 - s;
        }

        return ris;
    }
}

double UDPRechargeGameTheory::calculateDischargeProb(void){
    if (!sb->isCharging()) {
        return 0.0;
    }
    else {
        double ris;
        if (variableP) {

            int numberNodes = this->getParentModule()->getVectorSize();
            double estimatedTimeInRecharging;
            double energyToUse;
            double timeCalcNum, timeCalcDen1, timeCalcDen2;
            double gPLUSt = getGamma() + getTheta();

            energyToUse = sb->getBatteryLevelAbs();
            switch (dischargeProbEnergyToUse) {
            case ENERGYMIN:
            default:
                energyToUse = getEmin(false, GLOBAL_KNOWLEDGE);
                break;
            case ENERGYMAX:
                energyToUse = getEmax(false, GLOBAL_KNOWLEDGE);
                break;
            case ENERGYAVG:
                energyToUse = getEavg(false, GLOBAL_KNOWLEDGE);
                break;
            }

            timeCalcNum = energyToUse - gPLUSt;
            timeCalcDen1 = getAlpha() * ((double)(numberNodes - 1.0));
            timeCalcDen2 = ((double)(numberNodes - 1.0)) * gPLUSt;

            estimatedTimeInRecharging = timeCalcNum / (timeCalcDen1 + timeCalcDen2);

            if (exponential_dischargeProb_decay == 0) {
                estimatedTimeInRecharging = estimatedTimeInRecharging / temp_factorProbDischarge;
                ris = 1.0 / estimatedTimeInRecharging;
            }
            else {
                double timeInCharge = (simTime() - startRecharge).dbl();

                estimatedTimeInRecharging = estimatedTimeInRecharging * checkRechargeTimer;

                //fprintf(stderr, "timeInCharge: %lf and estimatedTimeInRecharging = %lf\n", timeInCharge, estimatedTimeInRecharging); fflush(stderr);

                if (timeInCharge >= estimatedTimeInRecharging){
                    ris = 1.0;
                }
                else {
                    ris = pow(timeInCharge / estimatedTimeInRecharging, exponential_dischargeProb_decay);
                }
            }

        }
        else {
            ris = constDischargeProb;
        }

        if (ris < 0) ris = 0;
        if (ris > 1) ris = 1;

        return ris;
    }
}


double UDPRechargeGameTheory::getGameTheoryC(void) {
    double ris = 0;

    if (variableC) {
        switch (constant_type) {
        case LINEARINCREASE:
        default:
            ris = getGameTheoryC_LinearIncrease();
            break;
        case LINEARINCREASECONSISTENT:
            ris = getGameTheoryC_LinearIncreaseConsistent();
            break;
        }
        //fprintf(stderr, "DEVSTIM: myE: %lf; avgE: %lf; ratio: %lf\n", sb->getBatteryLevelAbs(), eavg, eavg / sb->getBatteryLevelAbs()); fflush(stderr);
        //fprintf(stderr, "DEVSTIM: getGameTheoryC - e^2: %lf theta: %lf; gamma: %lf; alpha: %lf; beta: %lf; ris: %lf\n", e, t, g, a, b, ris); fflush(stderr);
    }
    else {
        double d = (getTheta() + getGamma()) / (getAlpha() + getBeta());

        //fprintf(stderr, "DEVSTIM: theta: %lf; gamma: %lf; alpha: %lf; beta: %lf\n", getTheta(), getGamma(), getAlpha(), getBeta()); fflush(stderr);

        ris = 1.0 - d;
    }

    if (ris > 1) ris = 1;
    if (ris < 0) ris = 0;

    return ris;
}

double UDPRechargeGameTheory::getGameTheoryC_LinearIncrease(void) {

    double utMeno, utPiuOk, utPiuFail;
    double ris = 0;
    double eMAX = getEmax(false, gameTheoryKnowledgeType);
    double eMIN = getEmin(false, gameTheoryKnowledgeType);
    double a = getAlpha();
    double b = getBeta();
    double t = getTheta();
    double g = getGamma();
    double myE = sb->getBatteryLevelAbs();
    //double e = (((myE - eMIN) / (eMAX - eMIN)) * (1.0 - dicountminLINEAR4)) + 1.0;
    double e = 1;

    //if ((eMAX - eMIN) != 0) {
    //    e = (((eMAX - myE) / (eMAX - eMIN)) * (1.0 - linearIncreaseFactor)) + 1.0;
    //}
    //ris = (a + b - t - g) / ((e * (a + t + g)) + b - t - g);

    if ((eMAX - eMIN) != 0) {
        e = (eMAX - myE) / (eMAX - eMIN);
    }

    utMeno = -a;
    utPiuOk = b-g-t;
    utPiuFail = (-a-g-t) * (1.0 + (e * linearIncreaseFactor));

    ris = (utMeno - utPiuOk) / (utPiuFail - utPiuOk);

    //fprintf(stderr, "getGameTheoryC_LinearDiscount: %lf; alpha: %lf; beta: %lf; gamma: %lf; theta: %lf; myE: %lf; eMIN: %lf; eMAX: %lf; e: %lf\n",
    //        ris, a, b, g, t, myE, eMIN, eMAX, e); fflush(stderr);

    return ris;
}

double UDPRechargeGameTheory::getGameTheoryC_LinearIncreaseConsistent(void) {
    double ris = 0;
    double eMAX = getEmax(false, gameTheoryKnowledgeType);
    double eMIN = getEmin(false, gameTheoryKnowledgeType);
    double a = getAlpha();
    double b = getBeta();
    double t = getTheta();
    double g = getGamma();
    double myE = sb->getBatteryLevelAbs();
    //double e = (((myE - eMIN) / (eMAX - eMIN)) * (1.0 - dicountminLINEAR4)) + 1.0;
    double e = 1;

    if ((eMAX - eMIN) != 0) {
        //e = (((eMAX - myE) / (eMAX - eMIN)) * (1.0 - dicountminLINEAR4)) + 1.0;
        e = (eMAX - myE) / (eMAX - eMIN);
    }

    //ris = (a + b - t - g) / ((e * (a + t + g)) + b - t - g);

    double utMeno, utPiuOk, utPiuFail;

    if (((e + 1.0)/(1.0 - e)) < 0.0001) return 0.0000001;

    utMeno = -a;
    utPiuOk = b-g-t;
    //utPiuFail = (-a-g-t) + log((1.0 - e)/(e + 1.0));
    utPiuFail = (-a-g-t) * (1.0 + log((e + 1.0)/(1.0 - e)));

    ris = (utMeno - utPiuOk) / (utPiuFail - utPiuOk);

    //fprintf(stderr, "getGameTheoryC_LinearDiscount: %lf; alpha: %lf; beta: %lf; gamma: %lf; theta: %lf; myE: %lf; eMIN: %lf; eMAX: %lf; e: %lf\n",
    //        ris, a, b, g, t, myE, eMIN, eMAX, e); fflush(stderr);

    return ris;
}

double UDPRechargeGameTheory::getTheta(void) {
    return sb->getSwapLoose();
}

double UDPRechargeGameTheory::getGamma(void) {
    return sb->getSwapLoose();
}

double UDPRechargeGameTheory::getAlpha(void) {
    return sb->getDischargingFactor(checkRechargeTimer);
}

double UDPRechargeGameTheory::getBeta(void) {
    return sb->getChargingFactor(checkRechargeTimer);
}

double UDPRechargeGameTheory::getP(void) {
    return 1;
}

double UDPRechargeGameTheory::getEavg(bool activeOnly, GameTheoryKnowledge_Type scope) {
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
        for (auto it = neigh.begin(); it != neigh.end(); it++) {
            nodeInfo_t *act = &(it->second);
            sum += act->batteryLevelAbs;
        }
        nn = neigh.size() + 1;
    }
    else {
        error("Wrong knowledge scope");
    }

    return (sum / nn);
}

double UDPRechargeGameTheory::getEmax(bool activeOnly, GameTheoryKnowledge_Type scope) {
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
        for (auto it = neigh.begin(); it != neigh.end(); it++) {
            nodeInfo_t *act = &(it->second);
            double actBatt = act->batteryLevelAbs;
            if (actBatt > max) {
                max = actBatt;
            }
        }
    }
    else {
        error("Wrong knowledge scope");
    }

    return max;
}

double UDPRechargeGameTheory::getEmin(bool activeOnly, GameTheoryKnowledge_Type scope) {
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
        for (auto it = neigh.begin(); it != neigh.end(); it++) {
            nodeInfo_t *act = &(it->second);
            double actBatt = act->batteryLevelAbs;
            if (actBatt < min) {
                min = actBatt;
            }
        }
    }
    else {
        error("Wrong knowledge scope");
    }

    return min;
}

} /* namespace inet */