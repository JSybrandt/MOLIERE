#!/bin/bash

module load python gcc

./profileHypotheses.py -l examples/graph.labels -e examples/graph.edges -d examples/dijkFile -c examples/centrailityFile -o examples/out -V ./examples/VIEWS -D ./examples/DATA -v


# ./profileHypotheses_SNAP.py -l examples/graph.labels -e examples/graph.edges -d examples/dijkFile -c examples/centrailityFile -o examples/out -v
