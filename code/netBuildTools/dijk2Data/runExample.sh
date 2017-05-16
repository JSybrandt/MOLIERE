#!/bin/bash

module load python
module load sqlite

./dijk2Data.py -l /zfs/safrolab/users/jsybran/moliere/data/network/final.labels -p ./example/miniQuery.dijkstra -o example/out/ -a ./example/miniAbstracts.txt -v

