#!/bin/bash
module load gcc
./runDijkstra -g example/testGraph.edges -s 0 -o example/output.txt -v -n 15
