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

#include <simplebattery/SimpleBattery.h>
#include <vector>
#include <map>
#include <list>
#include <iomanip>      // std::setprecision

#include "inet/applications/udpapp/UDPBasicApp.h"

#include "inet/mobility/single/VirtualSpringMobility.h"
#include "inet/applications/base/ApplicationPacketRecharge_m.h"

namespace inet {

class INET_API UDPRechargeBasic : public UDPBasicApp {
public:

    typedef enum {
        ANALYTICAL,
        ROUNDROBIN,
        STIMULUS,
        PROBABILISTIC
    } Scheduling_Type;

public:

    UDPRechargeBasic() {}
    virtual ~UDPRechargeBasic();
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_UDPAPP_UDPRECHARGEBASIC_H_ */
