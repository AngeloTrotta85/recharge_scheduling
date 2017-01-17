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

package inet.applications.udpapp;

simple UDPRechargeStimulus extends UDPRechargeBasic
{
    parameters:
        
        double stimulusExponent = default(1);									// stimulus exponent
        int numRechargeSlotsStimulusZeroNeigh = default(3);						// default number of recharge slots if no neighbours
        string chargeLengthType @enum("MIN", "MAX", "AVG") = default("AVG");	// aggregation method (neighbourood energy) to use in recharge time calculation
        bool stationANDnodeKNOWN = default(false);								// knowledge of the numero of the uavs
        double chargeTimeOthersNodeFactor = default(0.0);						// factor to equalize the recharging time between uavs
        bool makeLowEnergyFactorCurves = default(true);							// method for calculating the stimulus
        
        bool useProbabilisticDischarge = default(false);						// if true, use (1/T) as probability of discharging (T=calculated recharge time)
        bool useQuadraticProbabilisticDischarge = default(false);				// if true, use (t/T)^2 as probability of discharging (T=calculated recharge time; t=time from start charging)
        
        @display("i=block/app");
        @class(UDPRechargeStimulus);
}