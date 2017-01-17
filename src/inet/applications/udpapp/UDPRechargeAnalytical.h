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
  virtual void checkCentralizedRecharge(void);
  virtual void checkCentralizedRechargeGroup(groupInfo_t *actGI);
  virtual bool checkScheduleFeasibilityGroup(groupInfo_t *actGI);

  virtual bool decideRechargeScedulingGroup(groupInfo_t *actGI);
  virtual bool decideRechargeScedulingGroupRR(groupInfo_t *actGI);
  virtual void decideRechargeScedulingGroupLast(groupInfo_t *actGI);

  void putNodeInCharging(int addr);
  void putNodeInDischarging(int addr);

  void updateBatteryVals(std::list<nodeAlgo_t> *list);
  int getNodeWithMaxEnergy(groupInfo_t *gi, double &battVal);
  int getNodeWithMinEnergy(groupInfo_t *gi, double &battVal);

  void printChargingInfo(void);
  void printChargingInfo(std::ostream &ss, const char *str);
  void printChargingInfo(const char *str);

public:
  virtual ~UDPRechargeAnalytical();

  virtual double calculateRechargeProb(void) {return 0;}
  virtual double calculateDischargeProb(void) {return 0;}

private:
    std::list<groupInfo_t> groupList;

    Scheduling_Type st;

    int roundrobinRechargeSize;
    char logFile[256];
    bool printAnalticalLog;

};

} /* namespace inet */

#endif /* INET_APPLICATIONS_UDPAPP_UDPRECHARGEANALYTICAL_H_ */
