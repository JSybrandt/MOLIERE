#!/bin/bash

module load python gcc

./findPaths.py -p ./examples/problem.txt \
               -e ./examples/graph.edges \
               -l ./examples/graph.labels \
               -o ./examples/out.txt
