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

#ifndef INET_APPLICATIONS_UDPAPP_UDPRECHARGEPROBABILISTIC_H_
#define INET_APPLICATIONS_UDPAPP_UDPRECHARGEPROBABILISTIC_H_

#include "inet/common/INETDefs.h"

#include <vector>
#include <map>
#include <list>
#include <iomanip>      // std::setprecision

#include "inet/applications/udpapp/UDPRechargeBasic.h"

namespace inet {

class INET_API UDPRechargeProbabilistic : public UDPRechargeBasic {

public:

    typedef enum {
        PERSONAL_KNOWLEDGE,
        LOCAL_KNOWLEDGE,
        GLOBAL_KNOWLEDGE
    } ProbabilisticKnowledge_Type;

protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual double calculateStaticDischargeProbability(void);

    double getEmax(bool activeOnly, ProbabilisticKnowledge_Type scope);
    double getEmin(bool activeOnly, ProbabilisticKnowledge_Type scope);
    double getEavg(bool activeOnly, ProbabilisticKnowledge_Type scope);

    double calculateEstimatedTimeInRecharging(void);
public:
    virtual ~UDPRechargeProbabilistic();


    virtual double calculateRechargeProb(void);
    virtual double calculateDischargeProb(void);

protected:
    bool useDischargeProbability;
    bool useOptimalRechargeTime;
    int chargeSlots;
    ProbabilisticKnowledge_Type probKnowledgeType;

    int countRechargeSlot;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_UDPAPP_UDPRECHARGEPROBABILISTIC_H_ */
