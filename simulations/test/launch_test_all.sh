#!/bin/bash

if [ $# -lt 1 ]; then
	echo "(1) Usage $0 proc_in_parallel"
	exit
fi

NPROC=`nproc`
NPARALLEL=$1

if [ $NPARALLEL -gt $NPROC ]; then
	echo "(1) Usage $0 proc_in_parallel"
	echo "proc_in_parallel cannot be greater then $NPROC"
	exit
fi

for i in $(seq ${NPARALLEL})
do

        #./launch_test.sh ${i} ${NPARALLEL} "TestFullCentrANALYTICAL TestFullCentrROUNDROBIN TestFullCentrNORECHARGE" /media/angelo/BigLinux/Documents/OmnetSimulations/recharge_scheduler/TestFullLog &
        #./launch_test.sh ${i} ${NPARALLEL} "TestFullDistrPROBABILISTIC TestFullDistrSTIMULUS TestFullDistrGAMETHEORY" /media/angelo/BigLinux/Documents/OmnetSimulations/recharge_scheduler/TestFullLog &
        #./launch_test.sh ${i} ${NPARALLEL} "TestFullCentrANALYTICAL TestFullCentrROUNDROBIN TestFullCentrNORECHARGE TestFullDistrPROBABILISTIC TestFullDistrGAMETHEORY" /media/angelo/BigLinux/Documents/OmnetSimulations/recharge_scheduler/TestFullLog &
        #./launch_test.sh ${i} ${NPARALLEL} "TestFullDistrGAMETHEORY" /media/angelo/BigLinux/Documents/OmnetSimulations/recharge_scheduler/TestFullLog &
        #./launch_test.sh ${i} ${NPARALLEL} "TestFullDistrGAMETHEORY" /media/angelo/BigLinux/Documents/OmnetSimulations/recharge_scheduler/TestFullLog_GT &
        #./launch_test.sh ${i} ${NPARALLEL} "TestFullDistrPROBABILISTIC" /media/angelo/BigLinux/Documents/OmnetSimulations/recharge_scheduler/TestFullLog_PROB &
        #./launch_test.sh ${i} ${NPARALLEL} "TestOnlyGT" /media/angelo/BigLinux/Documents/OmnetSimulations/recharge_scheduler/TestLog &
        #./launch_test.sh ${i} ${NPARALLEL} "TestFullDistrPROBABILISTICoptim" /media/angelo/BigLinux/Documents/OmnetSimulations/recharge_scheduler/TestLogTmp &

	#./launch_test.sh ${i} ${NPARALLEL} "TestFullDistrGAMETHEORY_PERSONAL" /media/angelo/BigLinux/Documents/OmnetSimulations/recharge_scheduler/TestLogGT_PROB &
	#./launch_test.sh ${i} ${NPARALLEL} "TestFullDistrGAMETHEORY_GLOBAL" /media/angelo/BigLinux/Documents/OmnetSimulations/recharge_scheduler/TestLogGT_GLOBAL &
	
	#./launch_test.sh ${i} ${NPARALLEL} "TestFullGammaDeltaCentrANALYTICAL TestFullGammaDeltaCentrNORECHARGE TestFullGammaDeltaDistrGAMETHEORY TestFullGammaDeltaDistrPROBABILISTIC" /media/angelo/BigLinux/Documents/OmnetSimulations/recharge_scheduler/TestFullGammaDeltaLog &	
	
	#./launch_test.sh ${i} ${NPARALLEL} "TestFullEinitCentrANALYTICAL TestFullEinitCentrNORECHARGE TestFullEinitDistrGAMETHEORY TestFullEinitDistrPROBABILISTIC TestFullHCentrANALYTICAL TestFullHCentrNORECHARGE TestFullHDistrGAMETHEORY TestFullHDistrPROBABILISTIC" /media/angelo/BigLinux/Documents/OmnetSimulations/recharge_scheduler/TestFullEinitLog &
	
	./launch_test.sh ${i} ${NPARALLEL} "TestFullAlphaBetaCentrNORECHARGE TestFullAlphaBetaCentrANALYTICAL TestFullAlphaBetaDistrPROBABILISTIC  TestFullAlphaBetaDistrGAMETHEORY" /media/angelo/BigLinux/Documents/OmnetSimulations/recharge_scheduler/TestFullAlphaBeta &

	#sleep 1
done

wait


