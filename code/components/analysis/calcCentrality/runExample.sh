#!/bin/bash

module load gcc
module load python


./calcCentrality.py -g examples/graph.edges -v -o examples/test.out
