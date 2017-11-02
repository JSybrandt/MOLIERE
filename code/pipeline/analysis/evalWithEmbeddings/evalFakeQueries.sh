#!/bin/bash
#PBS -N evcos
#PBS -l select=1:ncpus=24:mem=100gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/evcos.out
#PBS -e /home/jsybran/jobLogs/evcos.err
#PBS -M jsybran@clemson.edu
#PBS -m ea
# the above is a default PBS header

module load gcc gnu-parallel

# Place us in the working directory
if [ -z "$PBS_O_WORKDIR" ]; then
  echo "Must be submitted through PBS from home directory"
  exit 1
fi
cd $PBS_O_WORKDIR

# Identify the project home dir
if [ -z "$PROJ_HOME" ]; then
  echo "searching for moliere home directory"
  PROJ_HOME=$(pwd | grep -o .*moliere)
  if [ "$PROJ_HOME" = "" ]; then
    echo "Failed to find project home"
    exit 1
  else
    echo "Found $PROJ_HOME"
  fi
fi

# add project tools to path
PATH=$PATH:$PROJ_HOME/code/components/links

# data expected to be here
DATA=$PROJ_HOME/data
RES=$PROJ_HOME/results

TOPIC_DIR=$PROJ_HOME/results/validation/2010/VIEW_FAKE

CUID_VEC=$PROJ_HOME/data/yearlySubsets/2010/fastText/umls.data
NGRAM_VEC=$PROJ_HOME/data/yearlySubsets/2010/fastText/canon.retrained.vec

OUT=$RES/validation/2010/euclidian.fix.fake.ev

rm $OUT

evTopic(){
  f=$1
  if [ -f "$TOPIC_DIR/$f" ]; then
    SOURCE=$(awk 'BEGIN{FS="---"}{print $1}' <<< $f)
    TARGET=$(awk 'BEGIN{FS="---"}{print $2}' <<< $f)
    echo "$SOURCE $TARGET"
    evalWithEmbeddings -m $TOPIC_DIR/$f \
                       -n $NGRAM_VEC \
                       -c $CUID_VEC \
                       -s $SOURCE \
                       -t $TARGET \
                       -e \
                       >> $OUT
  fi
}

export -f evTopic
export TOPIC_DIR=$TOPIC_DIR
export OUT=$OUT
export CUID_VEC=$CUID_VEC
export NGRAM_VEC=$NGRAM_VEC
parallel -j 16 evTopic ::: $(ls $TOPIC_DIR)


#usage: ../../../components/analysis/evalWithEmbeddings/evalWithEmbeddings --topicModel=string --sourceLabel=string --targetLabel=string [options] ...
#options:
  #-m, --topicModel     Topic model from VIEW_FILES (string)
  #-n, --ngramVecs      ngram vector file (string [=])
  #-p, --pmidVecs       pmid vector file (string [=])
  #-c, --cuidVecs       cuid vector file (string [=])
  #-s, --sourceLabel    Source Label (string)
  #-t, --targetLabel    Target Label (string)
  #-v, --verbose        Output debug info.
  #-?, --help           print this message

