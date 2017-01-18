#!/bin/bash

if [ $# -lt 3 ]; then
	echo "(1) Usage $0 runIndex[1..4] totalRUN[1..4] \"c1 c2 ... cn\""
	exit
fi

if [ $1 -ge 5 ] && [ $1 -le 0 ]; then
	echo "(2) Usage $0 runIndex[1..4] totalRUN[1..4] \"c1 c2 ... cn\""
	exit
fi

if [ $1 -gt $2 ]; then
	echo "(3) Usage $0 runIndex[1..4] totalRUN[1..4] \"c1 c2 ... cn\""
	echo "runIndex[1..4] must be less then or equal to totalRUN"
	exit
fi

INDEX_USER=$1
INDEX_USER=$((INDEX_USER-1))
TOTAL_RUNS=$2
CONFIGS_INI=`opp_run -a omnetpp.ini | grep "^Config" | awk '{ print substr($2, 1, length($2)-1) ";" $3}'`
CONFIGS_USER=$3

#echo "CONFIGS_USER: $CONFIGS_USER"
#echo "CONFIGS_INI: $CONFIGS_INI"

for cu in $CONFIGS_USER; do
	for ci in $CONFIGS_INI; do
		#ci_name=$(echo $ci | tr ";" " ")
		ci_name=$(echo $ci | awk -F';' '{print $1}')
		ci_ns=$(echo $ci | awk -F';' '{print $2}')
		#echo $ci_name
		#echo $ci_ns
		#echo $ci
		#echo $cu

		if [ "$ci_name" = "$cu" ]; then 
			echo $ci_name
			#echo $cu
			#echo "STRINGS Matched"
			
			IDX=$INDEX_USER
			
			while [ $IDX -lt $ci_ns ]
			do
				echo "INDEX: $IDX"
				
				COMM="../../src/recharge_scheduling -r ${IDX} -u Cmdenv -c ${ci_name} -n ../../src:..:../../../../../../media/angelo/BigLinux/Programs/OMNeT++/omnetpp-5.0/samples/inet/examples:../../../../../../media/angelo/BigLinux/Programs/OMNeT++/omnetpp-5.0/samples/inet/src:../../../../../../media/angelo/BigLinux/Programs/OMNeT++/omnetpp-5.0/samples/inet/tutorials:../../../inet_ext/src:../../../inet_ext/simulations:../../../virtual_spring/examples:../../../virtual_spring/src -l ../../../../../../media/angelo/BigLinux/Programs/OMNeT++/omnetpp-5.0/samples/inet/src/INET -l ../../../inet_ext/src/inet_ext -l ../../../virtual_spring/src/virtual-spring --debug-on-errors=false omnetpp.ini"
				
				#echo "$COMM"
				$COMM
				
				IDX=$((IDX+TOTAL_RUNS))				
			done						
			
			
		fi
	done
done

#../../src/recharge_scheduling -r 2 -u Cmdenv -c TestBaseGAMETHEORY -n ../../src:..:../../../../../../media/angelo/BigLinux/Programs/OMNeT++/omnetpp-5.0/samples/inet/examples:../../../../../../media/angelo/BigLinux/Programs/OMNeT++/omnetpp-5.0/samples/inet/src:../../../../../../media/angelo/BigLinux/Programs/OMNeT++/omnetpp-5.0/samples/inet/tutorials:../../../inet_ext/src:../../../inet_ext/simulations:../../../virtual_spring/examples:../../../virtual_spring/src -l ../../../../../../media/angelo/BigLinux/Programs/OMNeT++/omnetpp-5.0/samples/inet/src/INET -l ../../../inet_ext/src/inet_ext -l ../../../virtual_spring/src/virtual-spring --debug-on-errors=false omnetpp.ini

