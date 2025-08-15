#!/bin/bash

SEED=$1
for ELEM in 128 256 512 1024 2048 4096 8192 16384 32768 65536
do
    ./gen $ELEM $SEED
    echo "N${ELEM}.txt is generated with seed $SEED"
done
