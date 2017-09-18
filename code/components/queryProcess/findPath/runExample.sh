#!/bin/bash
module load gcc
module load openmpi
PROJ_HOME=$(pwd | grep -o .*/moliere)
GRAPH=$PROJ_HOME/data/network/final.bin.edges
LABELS=$PROJ_HOME/data/network/final.labels
WORD_VEC=$PROJ_HOME/data/fastText/canon.vec
PMID_VEC=$PROJ_HOME/data/fastText/centroids.data
UMLS_VEC=$PROJ_HOME/data/fastText/umls.data
SOURCE_WORD="venlafaxine"
SOURCE_IDX=$( grep -nwm1 $SOURCE_WORD $LABELS | awk 'BEGIN{FS=":"}{print $1-1}')
TARGET_WORD="htr1a"
TARGET_IDX=$( grep -nwm1 $TARGET_WORD $LABELS | awk 'BEGIN{FS=":"}{print $1-1}')
OUT="./out.txt"

echo ./findPath -g $GRAPH \
                -l $LABELS \
                -s $SOURCE_IDX \
                -t $TARGET_IDX \
                -V $WORD_VEC \
                -P $PMID_VEC \
                -U $UMLS_VEC \
                -e 1.5 \
                -o $OUT
./findPath -g $GRAPH \
           -l $LABELS \
           -s $SOURCE_IDX \
           -t $TARGET_IDX \
           -V $WORD_VEC \
           -P $PMID_VEC \
           -U $UMLS_VEC \
           -e 1.5 \
           -o $OUT 
