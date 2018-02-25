#!/bin/bash

TOPIC_DIR="/zfs/safrolab/users/jsybran/moliere/results/validation/take1/2010/VIEW_FAKE"

function run(){
  FILE=$1
  BASE_NAME=`basename $FILE`
  OUT_DIR=$2
  F_TEXT_DIR="/zfs/safrolab/users/jsybran/moliere/data/yearlySubsets/2010/fastText"
  N_GRAM_VEC=$F_TEXT_DIR/canon.vec
  PMID_VEC=$F_TEXT_DIR/centroids.data
  UMLS_VEC=$F_TEXT_DIR/umls.data

  SOURCE=`awk 'BEGIN{FS="---"}{print $1}' <<< $BASE_NAME`
  TARGET=`awk 'BEGIN{FS="---"}{print $2}' <<< $BASE_NAME`
  ./evalTwE -m $FILE \
            -n $N_GRAM_VEC \
            -p $PMID_VEC \
            -c $UMLS_VEC \
            -s $SOURCE \
            -t $TARGET \
            -v > $OUT_DIR/$BASE_NAME
}
 export -f run
 parallel --env TOPIC_DIR --sshloginfile $PBS_NODEFILE --progress "run {} ./REAL_REDO" ::: $TOPIC_DIR/*

# usage: ./evalTwE --topicModel=string --sourceLabel=string --targetLabel=string [options] ...
# options:
#   -m, --topicModel     Topic model from VIEW_FILES (string)
#   -n, --ngramVecs      ngram vector file (string [=])
#   -p, --pmidVecs       pmid vector file (string [=])
#   -c, --cuidVecs       cuid vector file (string [=])
#   -s, --sourceLabel    Source Label (string)
#   -t, --targetLabel    Target Label (string)
#   -v, --verbose        Output debug info.
#   -?, --help           print this message

