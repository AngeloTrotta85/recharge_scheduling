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

#ifndef INET_APPLICATIONS_UDPAPP_UDPRECHARGEBASIC_H_
#define INET_APPLICATIONS_UDPAPP_UDPRECHARGEBASIC_H_

#include "inet/common/INETDefs.h"

#include <vector>
#include <map>
#include <list>
#include <iomanip>      // std::setprecision

#include "inet/applications/udpapp/UDPBasicApp.h"

#include <simplebattery/SimpleBattery.h>
#include "inet/mobility/single/VirtualSpringMobility.h"
#include "inet/applications/base/ApplicationPacketRecharge_m.h"
#include "inet/applications/base/ApplicationPacketPosition_m.h"

#define N_PERCENTAGE_COVERAGE 20

namespace inet {

class INET_API UDPRechargeBasic : public UDPBasicApp {

public:

    typedef struct {
        L3Address addr;
        int appAddr;
        double rcvPow;
        double rcvSnr;
        Coord pos;

        simtime_t timestamp;

        double batteryLevelAbs;
        double batteryLevelPerc;
        double coveragePercentage;
        double leftLifetime;
        int nodeDegree;
        double inRechargeT;
        double gameTheoryC;
        double gameTheoryPC;
        bool recharging;

        bool hasPosInfo;
        bool hasEnergyInfo;
    } nodeInfo_t;

protected:
  virtual int numInitStages() const override { return NUM_INIT_STAGES; }
  virtual void initialize(int stage) override;
  virtual void handleMessageWhenUp(cMessage *msg) override;
  virtual void finish(void) override;
  virtual void checkAlive(void);

  virtual ApplicationPacketRecharge *generatePktToSend(const char *name, bool goingToRecharge);
  virtual ApplicationPacketPosition *generatePktPosToSend(const char *name);
  virtual void sendPacket();
  virtual void processPacket(cPacket *msg);
  virtual double calculateSendBackoff(void);
  virtual void sendRechargeMessage(void);
  virtual void sendPositionMessage(void);
  virtual bool checkRechargingStationFree(void);

  virtual double calculateInterDistance(double radious);
  virtual void updateVirtualForces(void);
  virtual void updateNeighbourhood(void);

  virtual void getFilteredNeigh(std::map<int, nodeInfo_t> &filteredNeigh);
  virtual int calculateNodeDegree(void);

  virtual double calculateRechargeTime(void){return 0;}
  virtual bool checkRecharge(void);
  virtual bool checkDischarge(void);

  virtual void make1secStats(void);
  virtual void make5secStats(void);

  virtual double getFullCoverage(bool makeMap);

public:

    UDPRechargeBasic() {}
    virtual ~UDPRechargeBasic();

    virtual double getGameTheoryC(void) {return 0;}
    virtual double getGameTheoryPC(void) {return 0;}

    virtual double calculateRechargeProb(void);
    virtual double calculateDischargeProb(void);

    double getLooseRechargingChance() const {
        return looseRechargingChance;
    }

    double getEnergyToShare() const {
        return energyToShare;
    }

protected:
    L3Address myAddr;
    int myAppAddr;

    VirtualSpringMobility *mob = nullptr;
    power::SimpleBattery *sb = nullptr;

    std::map<int, nodeInfo_t> neigh;
    std::map<int, nodeInfo_t> filter_neigh;
    std::map<int, nodeInfo_t> neighBackupWhenRecharging;

    int rechargeLostAccess;
    int failedAttemptCount;
    int totalAttemptCount;
    double inRechargingTime;
    simtime_t startRecharge;
    simtime_t lastSawInRecharge;
    int numberNodesInSimulation;
    bool rechargeIsolation;
    bool saveNeighboursMsgs;
    double energyAtRecharge;
    bool makeCoverageLog;
    bool makeCoverageMap;
    int coverageMapIdx;
    std::string coverageMapFilename;

    double sumCoverageTot;
    double sumCoverageRatioTot;
    double sumCoverageRatioMaxTot;
    double countCoverage;

    double looseRechargingChance;

    double areaMaxToCoverage;
    double flightHeight;
    double sensorAngle;

    simtime_t timeToUpdateEnergy;
    simtime_t delayTimeToUpdateEnergy;
    double energyToShare;
    bool useEnergyToShare;
    bool shift5secTimer;


    bool firstCoveragePassPercent[N_PERCENTAGE_COVERAGE];
    int numberCoveragePassPercent[N_PERCENTAGE_COVERAGE];
    simtime_t endCoveragePassPercent[N_PERCENTAGE_COVERAGE];
    simtime_t endCoveragePassPercent2[N_PERCENTAGE_COVERAGE];
    simtime_t endCoveragePassPercent3[N_PERCENTAGE_COVERAGE];

    // messages
    cMessage *autoMsgRecharge = nullptr;
    cMessage *goToCharge = nullptr;
    cMessage *posMessage = nullptr;
    cMessage *backAfterLoose = nullptr;
    cMessage *stat1sec = nullptr;
    cMessage *stat5sec = nullptr;

    //parameters
    double checkRechargeTimer;
    double sensorRadious;
    Coord rebornPos;
    int chargingStationNumber;
    bool activateVirtualForceMovements;
    bool sendDifferentMessages;
    double positionMessageTimer;

    // statistical vectors
    cOutVector activeNodesVector;
    cOutVector rechargingNodesVector;
    cOutVector responseVector;
    cOutVector degreeVector;
    cOutVector fulldegreeVector;
    cOutVector energyVector;
    cOutVector energyVectorAllMean;
    cOutVector energyVectorAllMin;
    cOutVector energyVectorAllMax;
    cOutVector energyVectorAllMaxMinDiff;
    cOutVector energyVectorAllVar;
    cOutVector failedAttemptVector;
    cOutVector dischargeProbVector;
    cOutVector timeOfRechargeVector;
    cOutVector totalCoverageVector;
    cOutVector totalCoverageRatioVector;
    cOutVector totalCoverageRatioMaxVector;

    cOutVector hypotheticalDischargeProbVector;
    cOutVector hypotheticalResponseVector;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_UDPAPP_UDPRECHARGEBASIC_H_ */
