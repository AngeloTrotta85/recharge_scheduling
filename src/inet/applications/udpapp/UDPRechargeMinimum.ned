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


simple UDPRechargeMinimum extends UDPRechargeBasic
{
    parameters:
        bool useDischargeProbability = default(false);	// if true, use (1/chargeSlots) as probability of discharging
        bool useOptimalRechargeTime = default(false);	// if true, use optimal regarge as game theory
        int chargeSlots = default(1);					// number of recharging slots
        string minimKnowledgeType @enum("LOCAL_KNOWLEDGE", "GLOBAL_KNOWLEDGE", "PERSONAL_KNOWLEDGE") = default("PERSONAL_KNOWLEDGE");
        
        @display("i=block/app");
        @class(UDPRechargeMinimum);
}