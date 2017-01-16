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

#ifndef INET_APPLICATIONS_UDPAPP_UDPRECHARGEANALYTICAL_H_
#define INET_APPLICATIONS_UDPAPP_UDPRECHARGEANALYTICAL_H_

#include "inet/common/INETDefs.h"

#include <vector>
#include <map>
#include <list>
#include <iomanip>      // std::setprecision

#include "inet/applications/udpapp/UDPRechargeBasic.h"

namespace inet {

class INET_API UDPRechargeAnalytical : public UDPRechargeBasic {
public:

    typedef enum {
        ANALYTICAL,
        ROUNDROBIN
    } Scheduling_Type;

    typedef struct {
        int addr;
        int assignedRecharge;
        int executedRecharge;
        double energy;
        bool isCharging;
    } nodeAlgo_t;

    typedef struct {
        int chargingAppAddr;
        int swapNumber;
        std::list<nodeAlgo_t> nodeList;
    } groupInfo_t;

protected:
  virtual void initialize(int stage) override;
  virtual void handleMessageWhenUp(cMessage *msg) override;

  virtual void initCentralizedRecharge(void);
  virtual bool decideRechargeSceduling(void);

public:
    virtual ~UDPRechargeAnalytical();

private:
    std::list<groupInfo_t> groupList;

    Scheduling_Type st;

};

} /* namespace inet */

#endif /* INET_APPLICATIONS_UDPAPP_UDPRECHARGEANALYTICAL_H_ */
