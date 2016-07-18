#!/bin/sh

EXE="./run_sim"
DATADIR="data/tgt/*"
RESULT="result.txt"

for fn in $DATADIR
#for fn in data/tgt/x-ray
do
	$EXE -f $fn -p 1 >> $RESULT
	$EXE -f $fn -p 3 >> $RESULT

	for bs in 4096 8192 16384 32768
	do
		for ali in 131072 262144 524288
		do
			$EXE -f $fn -p 2 -b $bs -a $ali >> $RESULT
		done
	done
	for bs in 4096 8192 16384 32768
	do
		for ali in 131072 262144 524288
		do
			$EXE -f $fn -p 4 -b $bs -a $ali >> $RESULT
		done
	done
done

