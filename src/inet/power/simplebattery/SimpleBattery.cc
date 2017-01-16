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

#include <simplebattery/SimpleBattery.h>

namespace inet {
namespace power {

Define_Module(inet::power::SimpleBattery);

void SimpleBattery::initialize(int stage) {
    if (stage == INITSTAGE_LOCAL) {
        initialCapacity = par("initialCapacity");
        fullCapacity = par("nominalCapacity");
        batteryLevel = initialCapacity;
        updateInterval = par("updateInterval");

        chargingFactor = par("chargingFactor").doubleValue();
        dischargingFactor = par("dischargingFactor").doubleValue();
        flightHeight = par("flightHeight");
        swapHeightFactor = par("swapHeightFactor").doubleValue();

        lastBatteryCheck = simTime();

        bState = UNDEFINED_STATE;

        sumSwap = 0;
        sumPenalities = 0;

        autoMsg = new cMessage("batteryLevelUpdate");
        scheduleAt(simTime() + updateInterval, autoMsg);

        WATCH(batteryLevel);
        WATCH(bState);
    }

}

void SimpleBattery::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        if (msg == autoMsg){
            updateBatteryLevel();

            scheduleAt(simTime() + updateInterval, autoMsg);
        }
    }
}

void SimpleBattery::finish(void) {
    recordScalar("BATTERYSWAP", sumSwap);
    recordScalar("BATTERYPENALITIES", sumPenalities);
}

double SimpleBattery::getSwapLoose(void) {
    return (flightHeight * swapHeightFactor);
}

void SimpleBattery::setDoubleSwapPenality(void) {
    batteryLevel -= getSwapLoose() * 2.0;

    sumPenalities++;

    if(batteryLevel < 0) {
        batteryLevel = 0;
    }
}

void SimpleBattery::setState(batteryState bs) {
    batteryState old_bs = bState;

    updateBatteryLevel();

    bState = bs;

    if ((old_bs != bState) && (old_bs != UNDEFINED_STATE) && (bState != UNDEFINED_STATE)) {
        batteryLevel -= getSwapLoose();

        if(batteryLevel < 0) {
            batteryLevel = 0;
        }

        sumSwap++;
    }
}

void SimpleBattery::updateBatteryLevel(void) {

    double timePassed = (simTime() - lastBatteryCheck).dbl();

    if (bState == DISCHARGING) {
        batteryLevel -= dischargingFactor * timePassed;

        if(batteryLevel < 0) {
            batteryLevel = 0;
        }

        //EV << "BATTERY DISCHARGING. ACTUAL VALUE: " << batteryLevel << endl;
    }
    else if (bState == CHARGING){
        batteryLevel += chargingFactor * timePassed;

        if(batteryLevel > fullCapacity) {
            batteryLevel = fullCapacity;
        }

        //EV << "BATTERY CHARGING. ACTUAL VALUE: " << batteryLevel << endl;
    }

    lastBatteryCheck = simTime();
}

} /* namespace power */
} /* namespace inet */
