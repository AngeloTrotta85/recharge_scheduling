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
        //variableC = par("variableC").boolValue();
        //variableP = par("variableP").boolValue();
        linearIncreaseFactor = par("linearIncreaseFactor");
        constDischargeProb = par("constDischargeProb");
        exponential_dischargeProb_decay = par("exponential_dischargeProb_decay");
        temp_factorProbDischarge = par("temp_factorProbDischarge");
        useNewGameTheoryDischargeProb = par("useNewGameTheoryDischargeProb").boolValue();
        kappaMeno = par("kappaMeno");
        kappaPiu = par("kappaPiu");

        estimateDischargeProbVector.setName("EstimateDischargeProbVector");
        estimatedTimeInRechargingVector.setName("EstimatedTimeInRechargingVector");

        std::string gameTheoryKnowledgeType_str = par("gameTheoryKnowledgeType").stdstringValue();
        if (gameTheoryKnowledgeType_str.compare("LOCAL_KNOWLEDGE") == 0) {
            gameTheoryKnowledgeType = LOCAL_KNOWLEDGE;
        }
        else if (gameTheoryKnowledgeType_str.compare("GLOBAL_KNOWLEDGE") == 0) {
            gameTheoryKnowledgeType = GLOBAL_KNOWLEDGE;
        }
        else if (gameTheoryKnowledgeType_str.compare("PERSONAL_KNOWLEDGE") == 0) {
            gameTheoryKnowledgeType = PERSONAL_KNOWLEDGE;
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
        if (constType.compare("LINEARINCREASE") == 0) {
            constant_T_type = LINEARINCREASE;
        }
        else if (constType.compare("LINEARINCREASE2") == 0) {
            constant_T_type = LINEARINCREASE2;
        }
        else if (constType.compare("LINEARINCREASECONSISTENT") == 0) {
            constant_T_type = LINEARINCREASECONSISTENT;
        }
        else {
            error("Wrong \"varConstantType\" parameter: %s", constType.c_str());
        }

        std::string constPType = par("varPConstantType").stdstringValue();
        if (constPType.compare("LINEAR_P") == 0) {
            constant_P_type = LINEAR_P;
        }
        else if (constPType.compare("LINEAR_T") == 0) {
            constant_P_type = LINEAR_T;
        }
        else if (constPType.compare("NEW1") == 0) {
            constant_P_type = NEW1;
        }
        else if (constPType.compare("NEW2") == 0) {
            constant_P_type = NEW2;
        }
        else if (constPType.compare("NEW3") == 0) {
            constant_P_type = NEW3;
        }
        else if (constPType.compare("NEW4") == 0) {
            constant_P_type = NEW4;
        }
        else if (constPType.compare("NEW5") == 0) {
            constant_P_type = NEW5;
        }
        else if (constPType.compare("NEW6") == 0) {
            constant_P_type = NEW6;
        }
        else {
            fprintf(stderr, "varPConstantType: %s\n", constPType.c_str());fflush(stderr);
            error("Wrong \"varPConstantType\" parameter");
        }

        std::string dischargeEType = par("dischargeEstimationType").stdstringValue();
        if (dischargeEType.compare("ONE_OVER") == 0) {
            dischargeEstimationType = ONE_OVER;
        }
        else if (dischargeEType.compare("QUADRATIC_EST") == 0) {
            dischargeEstimationType = QUADRATIC_EST;
        }
        else {
            fprintf(stderr, "dischargeEstimationType: %s\n", dischargeEType.c_str());fflush(stderr);
            error("Wrong \"dischargeEstimationType\" parameter");
        }
    }
    else if (stage == INITSTAGE_LAST) {
        bonus = (getAlpha() + getBeta()) / 2.0;

        switch (gameTheoryKnowledgeType) {
        case GLOBAL_KNOWLEDGE:
        default:
            variableC = true;
            variableP = true;
            break;

        case LOCAL_KNOWLEDGE:
            variableC = true;
            variableP = true;
            break;

        case PERSONAL_KNOWLEDGE:
            variableC = false;
            variableP = true;
            saveNeighboursMsgs = false;
            break;
        }
    }
}

void UDPRechargeGameTheory::handleMessageWhenUp(cMessage *msg) {
    if (msg == autoMsgRecharge) {

    }
    else if (msg == goToCharge) {

    }

    UDPRechargeBasic::handleMessageWhenUp(msg);

    if (msg == goToCharge) {
        if (sb->isCharging()) {
            // make neigh backup
            neighBackupWhenRecharging.clear();
            for (auto it = filter_neigh.begin(); it != filter_neigh.end(); it++) {
                nodeInfo_t actBkp = it->second;
                neighBackupWhenRecharging[it->first] = actBkp;
            }
        }
    }
}

void UDPRechargeGameTheory::make5secStats(void) {
    UDPRechargeBasic::make5secStats();

    estimateDischargeProbVector.record(calculateMyDischargeProb(gameTheoryKnowledgeType));
    estimatedTimeInRechargingVector.record(calculateEstimatedTimeInRecharging(dischargeProbEnergyToUse));
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
                    //dischargeP = calculateDischargeProb();
                    dischargeP = calculateMyDischargeProb(gameTheoryKnowledgeType);
                    //fprintf(stderr, "Estimated Discharge Prob: %Lf \n", dischargeP);fflush(stderr);
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
                        power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", j)->getSubmodule("battery"));

                        if (!battN->isCharging()) {
                            // TAKE ONLY ACTIVE NODES
                            double hostC = hostj->getGameTheoryC();
                            long double ppp = (1.0 - hostC) / unomenoCi;
                            produttoria = produttoria * ppp;

                            //fprintf(stderr, "%Lf ", ppp);
                        }

                    }
                }
                else if (gameTheoryKnowledgeType == LOCAL_KNOWLEDGE) {
                    for (auto it = filter_neigh.begin(); it != filter_neigh.end(); it++) {
                        nodeInfo_t *act = &(it->second);

                        double hostC = act->gameTheoryC;
                        long double ppp = (1.0 - hostC) / unomenoCi;
                        produttoria = produttoria * ppp;

                        //fprintf(stderr, "ppp: %Lf (hostC: %lf) (unomenoCi: %lf)", ppp, hostC, unomenoCi);
                    }
                    //fprintf(stderr, "\n");fflush(stderr);

                    //fprintf(stderr, "Produttoria after neigh check: %Lf (unomenoCi: %lf)\n", produttoria, unomenoCi);fflush(stderr);
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
                    if (dischargeP > 0) {
                        nmeno1SquareRoot = powl(produttoria, 1.0 / (((long double) numberNodes) - 2.0));
                    }
                    else{
                        nmeno1SquareRoot = powl(produttoria, 1.0 / (((long double) numberNodes) - 1.0));
                    }
                }
                else if (gameTheoryKnowledgeType == LOCAL_KNOWLEDGE){
                    if (filter_neigh.size() > 0) {
                        //fprintf(stderr, "Produttoria alla fine: %Lf \n", produttoria);fflush(stderr);
                        nmeno1SquareRoot = powl(produttoria, 1.0 / ((long double) filter_neigh.size()));
                        //fprintf(stderr, "Risultato: %lf (neigh size: %d)\n\n", nmeno1SquareRoot, ((int)filter_neigh.size()));fflush(stderr);
                    }
                    else {
                        double c = (1.0 - getGameTheoryC()) / (calculateMyDischargeProb(gameTheoryKnowledgeType));
                        nmeno1SquareRoot = pow(c, 1.0 / (((double) numberNodes) - 1.0));
                    }
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
            //double c = (getGamma()+getTheta()) / (getAlpha() + getBeta());
            //if (variableP){
            //    c = c / constDischargeProb;
            //}
            //s = pow(c, 1.0 / (((double) numberNodes) - 1.0));

            double c = (1.0 - getGameTheoryC()) / (calculateMyDischargeProb(gameTheoryKnowledgeType));
            s = pow(c, 1.0 / (((double) numberNodes) - 1.0));

            ris = 1.0 - s;
        }

        return ris;
    }
}

/*
double UDPRechargeGameTheory::estimateDischargeProb(void) {
    double ris, timeInCharge, estimatedTimeInRecharging;

    if (gameTheoryKnowledgeType != LOCAL_KNOWLEDGE){
        switch (dischargeEstimationType) {
        case ONE_OVER:
        default:
            ris = 1.0 / calculateEstimatedTimeInRecharging(dischargeProbEnergyToUse);
            break;
        case QUADRATIC_EST:
            estimatedTimeInRecharging = calculateEstimatedTimeInRecharging(dischargeProbEnergyToUse);

            timeInCharge = (simTime() - lastSawInRecharge).dbl();

            estimatedTimeInRecharging = estimatedTimeInRecharging * checkRechargeTimer;

            //fprintf(stderr, "timeInCharge: %lf and estimatedTimeInRecharging = %lf\n", timeInCharge, estimatedTimeInRecharging); fflush(stderr);

            if (timeInCharge >= estimatedTimeInRecharging){
                ris = 1.0;
            }
            else {
                ris = pow(timeInCharge / estimatedTimeInRecharging, exponential_dischargeProb_decay);
            }
            break;
        }
    }
    else {
        ris = 1.0 / calculateEstimatedTimeInRecharging(dischargeProbEnergyToUse);
    }

    if (ris > 1.0) ris = 1.0;
    if (ris < 0.0) ris = 0.0;
    return ris;
}
*/

double UDPRechargeGameTheory::calculateEstimatedTimeInRecharging(DischargeProbEnergyToUse_Type etu) {
    double estimatedTimeInRecharging, energyToUse, timeCalcNum, timeCalcDen1, timeCalcDen2, gPLUSt;
    int numberNodes = this->getParentModule()->getVectorSize();

    //switch (dischargeProbEnergyToUse) {
    switch (etu) {
    case ENERGYMIN:
    default:
        energyToUse = getEmin(false, gameTheoryKnowledgeType);
        break;
    case ENERGYMAX:
        energyToUse = getEmax(false, gameTheoryKnowledgeType);
        break;
    case ENERGYAVG:
        energyToUse = getEavg(false, gameTheoryKnowledgeType);
        break;
    }

    gPLUSt = getGamma() + getTheta();

    timeCalcNum = energyToUse - gPLUSt;
    timeCalcDen1 = getAlpha() * ((double)(numberNodes - 1.0));
    timeCalcDen2 = ((double)(numberNodes - 1.0)) * gPLUSt;

    estimatedTimeInRecharging = timeCalcNum / (timeCalcDen1 + timeCalcDen2);

    return estimatedTimeInRecharging;
}

double UDPRechargeGameTheory::calculateTimePassedRatioFromEstimatedNoLimit(void) {
    double ris = 0.0;

    if (sb->isCharging()) {
        double estimatedTimeInRecharging = calculateEstimatedTimeInRecharging(dischargeProbEnergyToUse);
        double timeInCharge = (simTime() - startRecharge).dbl();

        estimatedTimeInRecharging = estimatedTimeInRecharging * checkRechargeTimer;

        //fprintf(stderr, "timeInCharge: %lf and estimatedTimeInRecharging = %lf\n", timeInCharge, estimatedTimeInRecharging); fflush(stderr);

        ris = timeInCharge / estimatedTimeInRecharging;
    }

    return ris;

}

double UDPRechargeGameTheory::calculateTimePassedRatioFromEstimated(DischargeProbEnergyToUse_Type etu) {
    double ris = 0.0;
    double timeInCharge;
    double estimatedTimeInRecharging = calculateEstimatedTimeInRecharging(etu);

    if (sb->isCharging()) {
        timeInCharge = (simTime() - startRecharge).dbl();
    }
    else {
        timeInCharge = (simTime() - lastSawInRecharge).dbl();

    }

    estimatedTimeInRecharging = estimatedTimeInRecharging * checkRechargeTimer;

    //fprintf(stderr, "timeInCharge: %lf and estimatedTimeInRecharging = %lf\n", timeInCharge, estimatedTimeInRecharging); fflush(stderr);

    if (timeInCharge >= estimatedTimeInRecharging){
        ris = 1.0;
    }
    else {
        //ris = pow(timeInCharge / estimatedTimeInRecharging, exponential_dischargeProb_decay);
        ris = timeInCharge / estimatedTimeInRecharging;
    }

    return ris;
}

double UDPRechargeGameTheory::calculateMyDischargeProb(GameTheoryKnowledge_Type gtk){
    double ris;
    if (variableP) {

        if (useNewGameTheoryDischargeProb) {
            long double nmeno1SquareRoot;
            long double produttoria = 1.0;
            long double probCi = getGameTheoryProbC();

            //fprintf(stderr, "upMenoMore:%lf < upPiuMore:%lf - upPiuZero:%lf < upMenoZero:%lf\n",
            //        calculateUPminusMore(),calculateUPplusMore(),calculateUPplusZero(),calculateUPminusZero());
            //fprintf(stderr, "probCi: %Lf \n", probCi);

            if (gtk == GLOBAL_KNOWLEDGE) {
                for (int j = 0; j < numberNodesInSimulation; j++) {
                    UDPRechargeGameTheory *hostj = check_and_cast<UDPRechargeGameTheory *>(this->getParentModule()->getParentModule()->getSubmodule("host", j)->getSubmodule("udpApp", 0));
                    power::SimpleBattery *battN = check_and_cast<power::SimpleBattery *>(this->getParentModule()->getParentModule()->getSubmodule("host", j)->getSubmodule("battery"));

                    if (!battN->isCharging()) {
                        //{
                        // TAKE ONLY ACTIVE NODES
                        double hostC = hostj->getGameTheoryC();
                        long double ppp = (1.0 - hostC) / probCi;
                        produttoria = produttoria * ppp;

                        //fprintf(stderr, "HC:%lf;TMP:%Lf;DIV:%Lf      ", hostC, produttoria, ppp);
                        //fprintf(stderr, "%Lf ", ppp);
                    }

                }
                //fprintf(stderr, "\n");fflush(stderr);
            }
            else if (gtk == LOCAL_KNOWLEDGE) {
                if (sb->isCharging()) {
                    for (auto it = neighBackupWhenRecharging.begin(); it != neighBackupWhenRecharging.end(); it++) {
                        nodeInfo_t *act = &(it->second);

                        double hostC = act->gameTheoryC;
                        long double ppp = (1.0 - hostC) / probCi;
                        produttoria = produttoria * ppp;
                    }

                }
                else {
                    for (auto it = filter_neigh.begin(); it != filter_neigh.end(); it++) {
                        nodeInfo_t *act = &(it->second);

                        double hostC = act->gameTheoryC;
                        long double ppp = (1.0 - hostC) / probCi;
                        produttoria = produttoria * ppp;
                    }
                }
            }
            else if (gtk == PERSONAL_KNOWLEDGE) {
                produttoria = powl(getGameTheoryC()/probCi, numberNodesInSimulation - 1.0);
            }
            else {
                error("Wrong knowledge scope");
            }

            //fprintf(stderr, "Prod:%Lf \n", produttoria);
            produttoria = produttoria * probCi;
            //fprintf(stderr, "Prod*CK:%Lf \n", produttoria);

            if (gtk == GLOBAL_KNOWLEDGE) {
                nmeno1SquareRoot = powl(produttoria, 1.0 / (((long double) numberNodesInSimulation) - 1.0));
            }
            else if (gtk == LOCAL_KNOWLEDGE){
                if (sb->isCharging()) {
                    nmeno1SquareRoot = powl(produttoria, 1.0 / ((long double) neighBackupWhenRecharging.size()));
                }
                else {
                    nmeno1SquareRoot = powl(produttoria, 1.0 / ((long double) filter_neigh.size()));
                }
            }
            else if (gtk == PERSONAL_KNOWLEDGE) {
                nmeno1SquareRoot = powl(produttoria, 1.0 / (((long double) numberNodesInSimulation) - 1.0));
            }
            else {
                error("Wrong knowledge scope");
            }

            fprintf(stderr, "RIS:%Lf \n\n", nmeno1SquareRoot);

            ris = nmeno1SquareRoot;
        }
        else {

            if (exponential_dischargeProb_decay == 0) {
                double estimatedTimeInRecharging = calculateEstimatedTimeInRecharging(dischargeProbEnergyToUse);
                estimatedTimeInRecharging = estimatedTimeInRecharging / temp_factorProbDischarge;
                ris = 1.0 / estimatedTimeInRecharging;
            }
            else {
                ris = calculateTimePassedRatioFromEstimated(dischargeProbEnergyToUse);
            }
        }

    }
    else {
        error("OBSOLETE CODE... TO REMOVE!");
        if (useNewGameTheoryDischargeProb) {
            int numberNodes = this->getParentModule()->getVectorSize();
            double unomenoc = 1.0 - getGameTheoryCNew();
            double cp = getGameTheoryProbC();

            ris = unomenoc / cp;
            ris = ris * pow(cp, 1.0 / (numberNodes - 1.0));
        }
        else {
            ris = constDischargeProb;
        }
    }

    if (ris < 0) ris = 0;
    if (ris > 1) ris = 1;

    return ris;

}

double UDPRechargeGameTheory::calculateDischargeProb(void){
    if (!sb->isCharging()) {
        return 0.0;
    }
    else {
        return calculateMyDischargeProb(gameTheoryKnowledgeType);
    }
}


double UDPRechargeGameTheory::getGameTheoryC(void) {
    double ris = 0;

    ris = getGameTheoryCNew();
    /*
    if (variableC) {
        switch (constant_T_type) {
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
    */

    return ris;
}

double UDPRechargeGameTheory::getGameTheoryCNew(void) {
    double utMenoFree, utMenoBusy, utPiuOk, utPiuFail;
    double ris = 0;

    utMenoFree = calculateUTminusFree();
    utMenoBusy = calculateUTminusBusy();
    utPiuOk = calculateUTplusOk();
    utPiuFail = calculateUTplusFail();

    ris = (utMenoFree - utPiuOk) / (utPiuFail - utPiuOk + utMenoFree - utMenoBusy);

    if (ris > 1) ris = 1;
    if (ris < 0) ris = 0;

    return ris;
}

double UDPRechargeGameTheory::getGameTheoryProbC(void) {
    double upMenoZero, upMenoMore, upPiuZero, upPiuMore;
    double ris = 0;

    upMenoZero = calculateUPminusZero();
    upMenoMore = calculateUPminusMore();
    upPiuZero = calculateUPplusZero();
    upPiuMore = calculateUPplusMore();

    ris = (upMenoMore - upPiuMore) / (upPiuZero - upPiuMore + upMenoMore - upMenoZero);

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
        if (sb->isCharging()){
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
            for (auto it = filter_neigh.begin(); it != filter_neigh.end(); it++) {
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
            for (auto it = filter_neigh.begin(); it != filter_neigh.end(); it++) {
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

double UDPRechargeGameTheory::calculateUTplusFail(void) {
    double valUTplusFail = 0;
    double eMAX = getEmax(false, gameTheoryKnowledgeType);
    double eMIN = getEmin(false, gameTheoryKnowledgeType);
    double a = getAlpha();
    //double b = getBeta();
    double t = getTheta();
    double g = getGamma();
    double myE = sb->getBatteryLevelAbs();
    //double e = (((myE - eMIN) / (eMAX - eMIN)) * (1.0 - dicountminLINEAR4)) + 1.0;
    double e = 1;
    if ((eMAX - eMIN) != 0) {
        e = (eMAX - myE) / (eMAX - eMIN);
    }

    if (variableC) {
        switch (constant_T_type) {
        case LINEARINCREASE:
        default:
            valUTplusFail = (-a-g-t) * (1.0 + (e * linearIncreaseFactor));
            break;
        case LINEARINCREASE2:
            valUTplusFail = (-a-g-t) * (kappaPiu + (e * linearIncreaseFactor));
            break;
        case LINEARINCREASECONSISTENT:
            valUTplusFail = (-a-g-t) * (1.0 + log((e + 1.0)/(1.0 - e)));;
            break;
        }
    }
    else {
        valUTplusFail = (-a-g-t);
    }

    return valUTplusFail;
}

double UDPRechargeGameTheory::calculateUTplusOk(void) {
    double valUTplusOk = 0;
    //double eMAX = getEmax(false, gameTheoryKnowledgeType);
    //double eMIN = getEmin(false, gameTheoryKnowledgeType);
    //double a = getAlpha();
    double b = getBeta();
    double t = getTheta();
    double g = getGamma();
    //double myE = sb->getBatteryLevelAbs();
    //double e = (((myE - eMIN) / (eMAX - eMIN)) * (1.0 - dicountminLINEAR4)) + 1.0;
    //double e = 1;
    //if ((eMAX - eMIN) != 0) {
    //    e = (eMAX - myE) / (eMAX - eMIN);
    //}

    if (variableC) {
        switch (constant_T_type) {
        case LINEARINCREASE:
        default:
            valUTplusOk = b-g-t;
            break;
        case LINEARINCREASE2:
            valUTplusOk = b-g-t;
            break;
        case LINEARINCREASECONSISTENT:
            valUTplusOk = b-g-t;
            break;
        }
    }
    else {
        valUTplusOk = b-g-t;
    }

    return valUTplusOk;
}

double UDPRechargeGameTheory::calculateUTminusBusy(void) {
    double valUTminusBusy = 0;
    //double eMAX = getEmax(false, gameTheoryKnowledgeType);
    //double eMIN = getEmin(false, gameTheoryKnowledgeType);
    double a = getAlpha();
    //double b = getBeta();
    //double t = getTheta();
    //double g = getGamma();
    //double myE = sb->getBatteryLevelAbs();
    //double e = (((myE - eMIN) / (eMAX - eMIN)) * (1.0 - dicountminLINEAR4)) + 1.0;
    //double e = 1;
    //if ((eMAX - eMIN) != 0) {
    //    e = (eMAX - myE) / (eMAX - eMIN);
    //}

    if (variableC) {
        switch (constant_T_type) {
        case LINEARINCREASE:
        default:
            valUTminusBusy = -a;
            break;
        case LINEARINCREASE2:
            valUTminusBusy = -a;
            break;
        case LINEARINCREASECONSISTENT:
            valUTminusBusy = -a;
            break;
        }
    }
    else {
        valUTminusBusy = -a;
    }

    return valUTminusBusy;
}

double UDPRechargeGameTheory::calculateUTminusFree(void) {
    double valUTminusFree = 0;
    double eMAX = getEmax(false, gameTheoryKnowledgeType);
    double eMIN = getEmin(false, gameTheoryKnowledgeType);
    double a = getAlpha();
    //double b = getBeta();
    //double t = getTheta();
    //double g = getGamma();
    double myE = sb->getBatteryLevelAbs();
    //double e = (((myE - eMIN) / (eMAX - eMIN)) * (1.0 - dicountminLINEAR4)) + 1.0;
    double e = 1;
    if ((eMAX - eMIN) != 0) {
        e = (eMAX - myE) / (eMAX - eMIN);
    }

    if (variableC) {
        switch (constant_T_type) {
        case LINEARINCREASE:
        default:
            valUTminusFree = -a;
            break;
        case LINEARINCREASE2:
            if (kappaMeno > 0){
                valUTminusFree = (-a) * (kappaMeno + (e * linearIncreaseFactor));
            }
            else {
                valUTminusFree = -a;
            }
            break;
        case LINEARINCREASECONSISTENT:
            valUTminusFree = -a;
            break;
        }
    }
    else {
        valUTminusFree = -a;
    }

    return valUTminusFree;
}


double UDPRechargeGameTheory::calculateUPplusZero(void) {
    double valUPplusZero = 0;
    double tr = 1 - calculateTimePassedRatioFromEstimated(ENERGYMIN);
    double r = calculateTimePassedRatioFromEstimated(ENERGYMIN);
    double eMAX = getEmax(false, gameTheoryKnowledgeType);
    double eMIN = getEmin(false, gameTheoryKnowledgeType);
    double a = getAlpha();
    //double b = getBeta();
    double t = getTheta();
    //double g = getGamma();
    double myE = sb->getBatteryLevelAbs();

    double e = 1;
    if ((eMAX - eMIN) != 0) {
        e = (eMAX - myE) / (eMAX - eMIN);
    }

    if (variableP) {
        switch (constant_P_type) {
        case LINEAR_P:
        default:
            valUPplusZero = (-a-t)*(1.0 + e)*0.5;
            break;
        case LINEAR_T:
            valUPplusZero = (-a-t)*(1.0 + tr);
            break;
        case NEW1:
            valUPplusZero = (-a-t);
            break;
        case NEW2:
            valUPplusZero = 0 - calculateTimePassedRatioFromEstimatedNoLimit();
            break;
        case NEW3:
            valUPplusZero = (-a * numberNodesInSimulation) - t;// + (1.0 * a * r);
            break;
        case NEW4:
            valUPplusZero = ( -(numberNodesInSimulation) * pow(r, exponential_dischargeProb_decay));
            break;
        case NEW5:
            valUPplusZero = ( -(numberNodesInSimulation) * pow(r, exponential_dischargeProb_decay));
            break;
        case NEW6:
            valUPplusZero = ( -(numberNodesInSimulation) * pow(r, exponential_dischargeProb_decay));
            break;
        }
    }
    else {
        valUPplusZero = ( -(numberNodesInSimulation) * pow(r, exponential_dischargeProb_decay));;
    }

    return valUPplusZero;
}

double UDPRechargeGameTheory::calculateUPplusMore(void) {
    double valUPplusMore = 0;
    double tr = 1 - calculateTimePassedRatioFromEstimated(ENERGYMIN);
    double r = calculateTimePassedRatioFromEstimated(ENERGYMIN);
    double eMAX = getEmax(false, gameTheoryKnowledgeType);
    double eMIN = getEmin(false, gameTheoryKnowledgeType);
    double a = getAlpha();
    double b = getBeta();
    double t = getTheta();
    double g = getGamma();
    double myE = sb->getBatteryLevelAbs();

    double e = 1;
    if ((eMAX - eMIN) != 0) {
        e = (eMAX - myE) / (eMAX - eMIN);
    }

    if (variableP) {
        switch (constant_P_type) {
        case LINEAR_P:
        default:
            valUPplusMore = (-a-t)*(1.0 - e)/numberNodesInSimulation;
            break;
        case LINEAR_T:
            valUPplusMore = (-a-t)*(tr)+(b*(1.0-tr));
            break;
        case NEW1:
            valUPplusMore = (1.0 + calculateTimePassedRatioFromEstimatedNoLimit());
            break;
        case NEW2:
            valUPplusMore = (2.0 + calculateTimePassedRatioFromEstimatedNoLimit());
            break;
        case NEW3:
            //valUPplusMore = (-a * (numberNodesInSimulation - 1.0)) + b - t;
            //valUPplusMore = ((-a-t) * (numberNodesInSimulation - 1.0)) + b - t - g + (bonus * 2.0 - e);
            valUPplusMore = ((-a) * (numberNodesInSimulation - 1.0)) + b - t - g + (bonus * 2.0 - e);
            break;
        case NEW4:
            valUPplusMore = 1.0;
            break;
        case NEW5:
            valUPplusMore = 1.0 + (3.0 * pow(1.0 - r, 2.0));
            break;
        case NEW6:
            valUPplusMore = 1.0 + (numberNodesInSimulation * pow(1.0 - r, 2.0));
            break;
        }
    }
    else {
        valUPplusMore = 1.0;
    }

    return valUPplusMore;
}

double UDPRechargeGameTheory::calculateUPminusZero(void) {
    double valUPminusZero = 0;
    //double tr = 1 - calculateTimePassedRatioFromEstimated();
    //double r = calculateTimePassedRatioFromEstimated();
    //double eMAX = getEmax(false, gameTheoryKnowledgeType);
    //double eMIN = getEmin(false, gameTheoryKnowledgeType);
    double a = getAlpha();
    double b = getBeta();
    //double t = getTheta();
    //double g = getGamma();
    //double myE = sb->getBatteryLevelAbs();

    //double e = 1;
    //if ((eMAX - eMIN) != 0) {
    //    e = (eMAX - myE) / (eMAX - eMIN);
    //}

    if (variableP) {
        switch (constant_P_type) {
        case LINEAR_P:
        default:
            valUPminusZero = b;
            break;
        case LINEAR_T:
            valUPminusZero = (b);//*(1.0 + tr);
            break;
        case NEW1:
            valUPminusZero = (b);//*(1.0 + tr);
            break;
        case NEW2:
            valUPminusZero = (2.0 + calculateTimePassedRatioFromEstimatedNoLimit());
            break;
        case NEW3:
            //valUPminusZero = (-a * (numberNodesInSimulation - 1.0)) + (b * (3.0 - r));
            //valUPminusZero = (-a * (numberNodesInSimulation - 1.0)) + b - (bonus * r);
            valUPminusZero = (-a * (numberNodesInSimulation - 1.0)) + b;
            break;
        case NEW4:
        case NEW5:
        case NEW6:
            //valUPminusZero = pow(1.0 - r, 2) * b;
            //valUPminusZero = 1.0 + pow(1.0 - r, 2);
            valUPminusZero = 0;
            break;
        }
    }
    else {
        valUPminusZero = 0;
    }

    return valUPminusZero;
}

double UDPRechargeGameTheory::calculateUPminusMore(void) {
    double valUPminusMore = 0;
    //double tr = 1 - calculateTimePassedRatioFromEstimated();
    double r = calculateTimePassedRatioFromEstimated(ENERGYMIN);
    double eMAX = getEmax(false, gameTheoryKnowledgeType);
    double eMIN = getEmin(false, gameTheoryKnowledgeType);
    double a = getAlpha();
    double b = getBeta();
    double t = getTheta();
    double g = getGamma();
    double myE = sb->getBatteryLevelAbs();

    double e = 1;
    if ((eMAX - eMIN) != 0) {
        e = (eMAX - myE) / (eMAX - eMIN);
    }

    if (variableP) {
        switch (constant_P_type) {
        case LINEAR_P:
        default:
            valUPminusMore = b - (numberNodesInSimulation * t * (1 + e));
            break;
        case LINEAR_T:
            valUPminusMore = 0;//(b * tr)-(numberNodesInSimulation * t * 3);
            break;
        case NEW1:
            valUPminusMore = (1.0 - calculateTimePassedRatioFromEstimatedNoLimit());//(b * tr)-(numberNodesInSimulation * t * 3);
            break;
        case NEW2:
            valUPminusMore = (1.0 - calculateTimePassedRatioFromEstimatedNoLimit());//(b * tr)-(numberNodesInSimulation * t * 3);
            break;
        case NEW3:
            //valUPminusMore = (((-a-g) * (numberNodesInSimulation - 1.0)) + b) * (5.0 - e);
            //valUPminusMore = ((-a-g) * (numberNodesInSimulation - 1.0)) + b;
            valUPminusMore = ((-a) * (numberNodesInSimulation - 1.0)) + b - g - (10.0 * bonus * (1.0 - r));
            break;
        case NEW4:
        case NEW5:
        case NEW6:
            //valUPminusMore = (-a * pow(1.0 - r, 2)) - a;
            //valUPminusMore = pow(1.0 - r, 2);
            valUPminusMore = 0;
            break;
        }
    }
    else {
        valUPminusMore = 0;
    }

    return valUPminusMore;
}


} /* namespace inet */
