#!/bin/bash
module load gcc
module load openmpi
PROJ_HOME=$(pwd | grep -o .*/moliere)
GRAPH=$PROJ_HOME/data/network/final.bin.edges
LABELS=$PROJ_HOME/data/network/final.labels
WORD_VEC=$PROJ_HOME/data/fastText/small_canon.vec
CENT_VEC=$PROJ_HOME/data/fastText/small_centroids.vec
SOURCE_WORD="find"
SOURCE_IDX=$( grep -nwm1 $SOURCE_WORD $LABELS | awk 'BEGIN{FS=":"}{print $1-1}')
TARGET_WORD="study"
TARGET_IDX=$( grep -nwm1 $TARGET_WORD $LABELS | awk 'BEGIN{FS=":"}{print $1-1}')

echo ./findPathWithSubset -g $GRAPH \
                     -l $LABELS \
                     -s $SOURCE_IDX \
                     -t $TARGET_IDX \
                     -V $WORD_VEC \
                     -C $CENT_VEC \
                     -e 1.5 \
                     -o "" \
                     -v
./findPathWithSubset -g $GRAPH \
                     -l $LABELS \
                     -s $SOURCE_IDX \
                     -t $TARGET_IDX \
                     -V $WORD_VEC \
                     -C $CENT_VEC \
                     -e 1.5 \
                     -o "" \
                     -v
