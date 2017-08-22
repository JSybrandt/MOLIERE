#!/bin/bash

module load gcc
./mergeNets -a example/graphA.edges -A example/graphA.labels -b example/graphB.edges -B example/graphB.labels -c example/ifidf.edges -r example/res.edges -R example/res.labels -v -x 5 -y 10 -z 20
