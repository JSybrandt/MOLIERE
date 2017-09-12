#!/bin/bash
module load gcc
module load openmpi
./findPathWithSubset -g "" -s 22281878 -t 22281875 -o "" -V ../../../../data/fastText/canon.vec -C ../../../../data/fastText/filtered_centroids.data -l ../../../../data/network/final.labels -e 10 -v
