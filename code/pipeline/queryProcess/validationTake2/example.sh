#!/bin/bash

module load gcc python
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
DATA=$PROJ_HOME/data/yearlySubsets/2010

VALIDATION_FILE=$PROJ_HOME/data/validationData/onlyPairs/real.pairs.txt

TMP_DIR=/scratch2/jsybran/validation_tmp/real
mkdir -p $TMP_DIR

WORD1="C0014025"
WORD2="C0033971"
runQuery.py -c $TMP_DIR -d $DATA -n 100 -e 1.4 -v $WORD1 $WORD2


#usage: runQuery.py [-h] [-c CACHE] [-d DATA_HOME] [-n NUM_TOPICS]
                   #[-e ELLIPSE_CONSTANT] [-m] [-r] [-s] [-v]
                   #wordA wordB

#positional arguments:
  #wordA
  #wordB

#optional arguments:
  #-h, --help            show this help message and exit
  #-c CACHE, --cache CACHE
                        #specifies where to store cached files.
  #-d DATA_HOME, --data_home DATA_HOME
                        #specifies an anternate data directory
  #-n NUM_TOPICS, --num_topics NUM_TOPICS
                        #specifies the number of topics to generate.
  #-e ELLIPSE_CONSTANT, --ellipse_constant ELLIPSE_CONSTANT
                        #size of ellipse optimization
  #-m, --move_here       move topic / analysis files to working dir
  #-r, --reconstruct     if set, do not reuse existing cached files.
  #-s, --skip_sanitize   if set, do not check for input in labels.
  #-v, --verbose         if set, run pipeline with verbose flags.

