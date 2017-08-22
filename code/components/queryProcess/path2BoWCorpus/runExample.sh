#!/bin/bash

module load python
module load sqlite

rm -rf ./example/out
mkdir ./example/out

./dijk2Data.py -l ./example/labels.txt -p ./example/miniQuery.dijkstra -o example/out/ -a ./example/miniAbstracts.txt -v

