#!/bin/bash

module load gcc openmpi

LABELS=../../../../data/network/final.labels
ABSTRACTS=../../../../data/processedText/abstracts.txt
OUT=./out
CLOUD=../../../../results/play/2016/cloud.txt

time ./cloud2Bag -c $CLOUD -l $LABELS -o $OUT -a $ABSTRACTS -v

