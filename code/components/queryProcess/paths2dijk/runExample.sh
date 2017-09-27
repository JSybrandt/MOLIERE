#!/bin/bash

module load gcc openmpi

GRAPH=../../../../data/network/final.bin.edges
LABELS=../../../../data/network/final.labels
OUT=./out
PATHS=../../../../results/validation/2010/allPaths.txt

./paths2Dijk -g $GRAPH -l $LABELS -p $PATHS -o $OUT -v

