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


import inet.applications.udpapp.UDPBasicApp;


simple UDPRechargeBasic extends UDPBasicApp
{
    parameters:
        double checkRechargeTimer @unit(s) = default(30s);
        double sensorRadious @unit(m) = default(50m);
        
        int chargingStationNumber = default(3);
        
        bool activateVirtualForceMovements = default(true);
        bool rechargeIsolation = default(true);
        bool makeCoverageLog = default(false);
        bool makeCoverageMap = default(false);
        string coverageMapFilename = default("/dev/null");
        
    	double flightHeight @unit(m) = default(50m);
    	double sensorAngle @unit(rad) = default(1.5708rad);  // PI/2
    	
    	bool sendDifferentMessages = default(false);
        double positionMessageTimer @unit(s) = default(1s);
        
        double delayTimeToUpdateEnergy @unit(s) = default(1s);
        bool useEnergyToShare = default(false);
        
        bool shift5secTimer = default(false);
                
        
        @display("i=block/app");
        @class(UDPRechargeBasic);
}
