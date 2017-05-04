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

#ifndef INET_APPLICATIONS_UDPAPP_UDPRECHARGEGAMETHEORY_H_
#define INET_APPLICATIONS_UDPAPP_UDPRECHARGEGAMETHEORY_H_

#include "inet/common/INETDefs.h"

#include <vector>
#include <map>
#include <list>
#include <iomanip>      // std::setprecision

#include "inet/applications/udpapp/UDPRechargeBasic.h"

namespace inet {

class INET_API UDPRechargeGameTheory : public UDPRechargeBasic {

public:

    typedef enum {
        PERSONAL_KNOWLEDGE,
        LOCAL_KNOWLEDGE,
        GLOBAL_KNOWLEDGE
    } GameTheoryKnowledge_Type;

    typedef enum {
        LINEARINCREASE,
        LINEARINCREASE2,
        LINEARINCREASE3,
        LINEARINCREASE4,
        LINEARINCREASE5,
        LINEARINCREASECONSISTENT
    } VarTConstant_Type;

    typedef enum {
        LINEAR_P,
        LINEAR_T,
        NEW1,
        NEW2,
        NEW3,
        NEW4,
        NEW5,
        NEW6,
        NEW7,
        NEW8,
        NEW9,
        NEW10,
        NEW11,
        NEW12
    } VarPConstant_Type;

    typedef enum {
        ENERGYMIN,
        ENERGYMAX,
        ENERGYAVG
    } DischargeProbEnergyToUse_Type;

    typedef enum {
        ONE_OVER,
        QUADRATIC_EST
    } DischargeEstimation_Type;

protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    double getGameTheoryC_LinearIncrease(void);
    double getGameTheoryC_LinearIncreaseConsistent(void);

    //virtual double estimateDischargeProb(void);
    virtual double estimateDischargeProb_LOCAL(void);
    virtual double calculateEstimatedTimeInRecharging(DischargeProbEnergyToUse_Type etu);
    virtual double calculateTimePassedRatioFromEstimatedNOBOUND(DischargeProbEnergyToUse_Type etu);
    virtual double calculateTimePassedRatioFromEstimated(DischargeProbEnergyToUse_Type etu);
    virtual double calculateTimePassedRatioFromEstimatedNoLimit(void);

    virtual double calculateUTplusFail(void);
    virtual double calculateUTplusFail(double energyToUse);
    virtual double calculateUTplusOk(void);
    virtual double calculateUTplusOk(double energyToUse);
    virtual double calculateUTminusBusy(void);
    virtual double calculateUTminusBusy(double energyToUse);
    virtual double calculateUTminusFree(void);
    virtual double calculateUTminusFree(double energyToUse);

    virtual double calculateUPplusZero(void);
    virtual double calculateUPplusZero(double energyToUse);
    virtual double calculateUPplusMore(void);
    virtual double calculateUPplusMore(double energyToUse);
    virtual double calculateUPminusZero(void);
    virtual double calculateUPminusZero(double energyToUse);
    virtual double calculateUPminusMore(void);
    virtual double calculateUPminusMore(double energyToUse);

    virtual void estimateAndUpdateNeighBackup(void);
    virtual double estimateLocalCWhenCharging(double oldEnergy);
    virtual double calculateMyDischargeProb(GameTheoryKnowledge_Type gtk);
    virtual double calculateMyDischargeProb(GameTheoryKnowledge_Type gtk, double energyToUse);

public:
    virtual ~UDPRechargeGameTheory();

    virtual double calculateRechargeProb(void);
    virtual double calculateDischargeProb(void);

    virtual double getGameTheoryC(double energyToUse);
    virtual double getGameTheoryC(void) override;
    virtual double getGameTheoryCNew(void);
    virtual double getGameTheoryCNew(double energyToUse);
    virtual double getGameTheoryProbC(void);
    virtual double getGameTheoryProbC(double energyToUse);

    double getTheta(void);
    double getGamma(void);
    double getAlpha(void);
    double getBeta(void);
    double getP(void);

    double getEavg(bool activeOnly, GameTheoryKnowledge_Type scope);
    double getEmax(bool activeOnly, GameTheoryKnowledge_Type scope);
    double getEmin(bool activeOnly, GameTheoryKnowledge_Type scope);

    virtual void make5secStats(void);

private:
    bool variableC;
    bool variableP;
    double linearIncreaseFactor;
    double constDischargeProb;
    double exponential_dischargeProb_decay;
    double temp_factorProbDischarge;
    bool useNewGameTheoryDischargeProb;
    double kappaMeno;
    double kappaPiu;
    bool useGlobalEstimationInLocal;
    bool useEnergyAtRechargeInDicharging;
    double personalConstantMultiplierC;
    bool useReverseE;
    double uTplusMultFactor;
    double coverageUtilityFactor;
    bool useUnoPRprob;
    double bonus;

    DischargeProbEnergyToUse_Type dischargeProbEnergyToUse;
    GameTheoryKnowledge_Type gameTheoryKnowledgeType;
    VarTConstant_Type constant_T_type;
    VarPConstant_Type constant_P_type;
    DischargeEstimation_Type dischargeEstimationType;

    cOutVector estimateDischargeProbVector;
    cOutVector estimatedTimeInRechargingVector;

    cOutVector prFactor;
    cOutVector prValueFactor;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_UDPAPP_UDPRECHARGEGAMETHEORY_H_ */
