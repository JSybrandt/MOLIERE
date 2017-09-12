#!/bin/bash
module load gcc
module load openmpi
./runAStar -g example/testGraph.edges -s 10 -T example/targets -o example/output.txt -n 15 -l example/testGraph.labels -C 2 -K 2 -N 3 -d example/test.vec
