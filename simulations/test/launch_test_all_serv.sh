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
        #./launch_test.sh ${i} ${NPARALLEL} "TestFullGammaDeltaCentrANALYTICAL_SERV TestFullGammaDeltaCentrNORECHARGE_SERV TestFullGammaDeltaDistrPROBABILISTIC_SERV TestFullGammaDeltaDistrGAMETHEORY_SERV" /home/angelo/git/recharge_scheduling/simulations/test/logs/TestFullDistrGammaDeltaLogs &
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
	
	#./launch_test_serv.sh ${i} ${NPARALLEL} "TestFullGammaDeltaCentrNORECHARGE_SERV TestFullGammaDeltaCentrANALYTICAL_SERV TestFullGammaDeltaDistrPROBABILISTIC_SERV TestFullGammaDeltaDistrGAMETHEORY_SERV TestFullEinitCentrNORECHARGE_SERV TestFullEinitCentrANALYTICAL_SERV TestFullEinitDistrPROBABILISTIC_SERV TestFullEinitDistrGAMETHEORY_SERV TestFullHCentrNORECHARGE_SERV TestFullHCentrANALYTICAL_SERV TestFullHDistrPROBABILISTIC_SERV TestFullHDistrGAMETHEORY_SERV TestFullNumUavCentrNORECHARGE_SERV TestFullNumUavCentrANALYTICAL_SERV TestFullNumUavDistrPROBABILISTIC_SERV TestFullNumUavDistrGAMETHEORY_SERV TestFullBeaconingEnergyCentrNORECHARGE_SERV TestFullBeaconingEnergyCentrANALYTICAL_SERV TestFullBeaconingEnergyDistrPROBABILISTIC_SERV TestFullBeaconingEnergyDistrGAMETHEORY_SERV TestFullBeaconingPositionCentrNORECHARGE_SERV TestFullBeaconingPositionCentrANALYTICAL_SERV TestFullBeaconingPositionDistrPROBABILISTIC_SERV TestFullBeaconingPositionDistrGAMETHEORY_SERV TestFullAlphaBetaCentrNORECHARGE_SERV TestFullAlphaBetaCentrANALYTICAL_SERV TestFullAlphaBetaDistrPROBABILISTIC_SERV TestFullAlphaBetaDistrGAMETHEORY_SERV" /home/angelo/git/recharge_scheduling/simulations/test/logs/TestFullLogs &
	./launch_test_serv.sh ${i} ${NPARALLEL} "TestFullGammaDeltaDistrGAMETHEORYbis_SERV" /home/angelo/git/recharge_scheduling/simulations/test/logs/TestFullLogs &

	#sleep 1
done

wait











