#!/bin/bash



NGRAM=/scratch2/jsybran/moliere/data/yearlySubsets/2010/fastText/canon.vec
CUID=/scratch2/jsybran/moliere/data/yearlySubsets/2010/fastText/umls.data
PMID=""

# Old metric said 0
SOURCE=C0001122
TARGET=C0007202
TOPIC=/scratch2/jsybran/moliere/results/validation/2010/VIEW/$SOURCE---$TARGET
./evalWithEmbeddings -m $TOPIC -n $NGRAM -c $CUID -s $SOURCE -t $TARGET

# Old metric said NAN
# SOURCE=C0000983
# TARGET=C0055819
# TOPIC=/scratch2/jsybran/moliere/results/validation/2010/VIEW/$SOURCE---$TARGET
# ./evalWithEmbeddings -m $TOPIC -n $NGRAM -c $CUID -s $SOURCE -t $TARGET -v

# Old metric said 1
# SOURCE=C0814022
# TARGET=C0021576
# TOPIC=/scratch2/jsybran/moliere/results/validation/2010/VIEW/$SOURCE---$TARGET
# ./evalWithEmbeddings -m $TOPIC -n $NGRAM -c $CUID -s $SOURCE -t $TARGET -v


#usage: ./evalWithEmbeddings --topicModel=string --sourceLabel=string --targetLabel=string [options] ... 
#options:
  #-m, --topicModel     Topic model from VIEW_FILES (string)
  #-n, --ngramVecs      ngram vector file (string [=])
  #-p, --pmidVecs       pmid vector file (string [=])
  #-c, --cuidVecs       cuid vector file (string [=])
  #-s, --sourceLabel    Source Label (string)
  #-t, --targetLabel    Target Label (string)
  #-v, --verbose        Output debug info.
  #-?, --help           print this message

