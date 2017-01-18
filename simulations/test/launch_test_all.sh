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
	./launch_test.sh ${i} ${NPARALLEL} "TestBaseANALYTICAL TestBaseROUNDROBIN TestBasePROBABILISTIC TestBaseSTIMULUS TestBaseGAMETHEORY" &
	sleep 1
done

wait


