#!/bin/bash
module load gcc
module load openmpi
./runDijkstra -g example/testGraph.edges -s 10 -T example/targets -o example/output.txt -v -n 15 -a 10 -C 2 -K 2 -N 3
