#!/bin/sh

EXE="./run_sim"
DATADIR="data/tgt/*"
RESULT="result.txt"


#$EXE -f data/tgt/osdb -p 1
#$EXE -f data/tgt/osdb -p 2 -b 16384 -a

#exit

for fn in $DATADIR
#for fn in data/tgt/osdb
do
	$EXE -f $fn -p 1 >> $RESULT
	$EXE -f $fn -p 3 >> $RESULT


	$EXE -f $fn -p 1 -a 8192 >> $RESULT
	$EXE -f $fn -p 3 -a 8192 >> $RESULT

#	for bs in 4096 8192 16384 32768
#	do
#		#for ali in 131072 262144 524288
#		for ali in 32768 65536 131072
#		do
#			$EXE -f $fn -p 2 -b $bs -a $ali >> $RESULT
#		done
#	done
#	for bs in 4096 8192 16384 32768
#	do
#		#for ali in 131072 262144 524288
#		for ali in 32768 65536 131072
#		do
#			$EXE -f $fn -p 4 -b $bs -a $ali >> $RESULT
#		done
#	done
#	echo >> $RESULT
done

