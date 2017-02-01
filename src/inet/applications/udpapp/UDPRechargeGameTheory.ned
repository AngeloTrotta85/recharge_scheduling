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

simple UDPRechargeGameTheory extends UDPRechargeBasic
{
    parameters:
        
        bool variableC = default(true);
        bool variableP = default(true);
        string gameTheoryKnowledgeType @enum("LOCAL_KNOWLEDGE", "GLOBAL_KNOWLEDGE") = default("GLOBAL_KNOWLEDGE");
        string varConstantType @enum("LINEARINCREASE", "LINEARINCREASECONSISTENT") = default("LINEARINCREASE");
        string varPConstantType @enum("LINEAR_P", "LINEAR_T") = default("LINEAR_P");
        string dischargeProbEnergyToUse @enum("ENERGYMIN", "ENERGYMAX", "ENERGYAVG") = default("ENERGYMIN");
        double linearIncreaseFactor = default(0.5);
        double constDischargeProb = default(1.0);
        double exponential_dischargeProb_decay = default(2.0);
        double temp_factorProbDischarge = default(2.0);
        bool useNewGameTheoryDischargeProb = default(false);
        
        @display("i=block/app");
        @class(UDPRechargeGameTheory);
}
