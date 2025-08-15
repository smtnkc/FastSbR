#!/bin/bash

DIR_ELEMENTS=elements
RESULTS_DIR=results

mkdir -p $DIR_ELEMENTS
mkdir -p $RESULTS_DIR

make clean all

chmod +x *.sh
