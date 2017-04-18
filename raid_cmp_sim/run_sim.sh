#!/bin/sh


LCOUNT=10000
RSEED=1

SIM="sim.py"
PYTHON="python"
RFILE='result.csv'

for buf in 16 32 64 128 256
do
    for dt in 0 1 2 3
    do
        ${PYTHON} ${SIM} -c ${LCOUNT} -s ${RSEED} -b ${buf} -d ${dt} >> ${RFILE}
    done
done
