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
        LOCAL_KNOWLEDGE,
        GLOBAL_KNOWLEDGE
    } GameTheoryKnowledge_Type;

    typedef enum {
        LINEARINCREASE,
        LINEARINCREASECONSISTENT
    } VarConstant_Type;

    typedef enum {
        ENERGYMIN,
        ENERGYMAX,
        ENERGYAVG
    } DischargeProbEnergyToUse_Type;

protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    double getGameTheoryC_LinearIncrease(void);
    double getGameTheoryC_LinearIncreaseConsistent(void);

public:
    virtual ~UDPRechargeGameTheory();

    virtual double calculateRechargeProb(void);
    virtual double calculateDischargeProb(void);

    virtual double getGameTheoryC(void) override;

    double getTheta(void);
    double getGamma(void);
    double getAlpha(void);
    double getBeta(void);
    double getP(void);

    double getEavg(bool activeOnly, GameTheoryKnowledge_Type scope);
    double getEmax(bool activeOnly, GameTheoryKnowledge_Type scope);
    double getEmin(bool activeOnly, GameTheoryKnowledge_Type scope);

private:
    bool variableC;
    bool variableP;
    double linearIncreaseFactor;
    double constDischargeProb;
    double exponential_dischargeProb_decay;
    double temp_factorProbDischarge;

    DischargeProbEnergyToUse_Type dischargeProbEnergyToUse;
    GameTheoryKnowledge_Type gameTheoryKnowledgeType;
    VarConstant_Type constant_type;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_UDPAPP_UDPRECHARGEGAMETHEORY_H_ */