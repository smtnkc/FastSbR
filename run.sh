#!/bin/bash

./prepare.sh

for ITR in 0 1 2 3 4 5 6 7 8 9
do
    echo "--------------- EXPERIMENT $ITR ---------------"
    SEED=$((ITR + 1000))
    ./generate.sh $SEED
    OUTPUT_FILE="results/stats_$SEED.csv"
    # write header
    echo "model,len,proc,itr,time" > $OUTPUT_FILE

    for ELEM in 128 256 512 1024 2048 4096 8192 16384 32768 65536
    do
        for NP in 2 4 8 16 32
        do
            mpirun -np $NP ./ms4 $ELEM $OUTPUT_FILE
            mpirun -np $NP ./fd4 $ELEM $OUTPUT_FILE
            mpirun -np $NP ./ms2 $ELEM $OUTPUT_FILE
            mpirun -np $NP ./fd2 $ELEM $OUTPUT_FILE
        done
    done

    for ELEM in 128 256 512 1024 2048 4096 8192 16384 32768 65536
    do
        ./sq4 $ELEM $OUTPUT_FILE
        ./sq2 $ELEM $OUTPUT_FILE
    done
done