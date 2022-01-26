#!/bin/bash

TESTS="mutex tas_lock tas_lock_relaxed ttas_lock amd_lock"

RATIO=$1
RATIO=${RATIO:='90'}
THREADS=$2
THREADS=${THREADS:='4'}
CYCLE="1000 500 250 100 50 25 10 5 2 1"
DURATION=10

echo "${TESTS}" | tr ' ' ','
for C in ${CYCLE}; do
    echo -n "${C},"
    for T in ${TESTS}; do
        ./${T} ${RATIO} ${C} ${DURATION} ${THREADS}
        echo -n ","
    done
    echo
done
