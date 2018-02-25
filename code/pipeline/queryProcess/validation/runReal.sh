#!/bin/bash
#PBS -N runRealREDO
#PBS -l select=1:ncpus=24:mem=100gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/runReal.out
#PBS -e /home/jsybran/jobLogs/runReal.err
#PBS -M jsybran@clemson.edu
#PBS -m ea
#PBS -J 0-99
# the above is a default PBS header

module load gcc python mpich
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

if [ -z "$PBS_ARRAY_INDEX" ]; then
  echo "must submit array job!"
  exit 1
fi


# add project tools to path
PATH=$PATH:$PROJ_HOME/code/components/links

# data expected to be here
DATA=$PROJ_HOME/data/yearlySubsets/2010

# VALIDATION_FILE=$PROJ_HOME/data/validationData/redo/real.txt
VALIDATION_FILE=/scratch2/jsybran/validation_tmp/redo/processed_real_pairs.txt

TMP_DIR=/scratch2/jsybran/validation_tmp/redo/real
mkdir -p $TMP_DIR

NUM_MACHINES=100

NUM_FILES=$(wc -l $VALIDATION_FILE | awk '{print $1}')

JOB_RANGE=$(($NUM_FILES / $NUM_MACHINES))

START=$(($PBS_ARRAY_INDEX * $JOB_RANGE))
END=$(($START + $JOB_RANGE))
if [ $PBS_ARRAY_INDEX == $(($NUM_MACHINES - 1)) ]; then
  END=$NUM_FILES
fi

echo "This job is responsible for files $START - $END"

for((i = $START; i < $END; i++)){
  LINE=$( sed -n "$(($i+1))"'p' $VALIDATION_FILE)
  WORD1=$(awk 'BEGIN{FS="|"}{print $1}' <<< $LINE)
  WORD2=$(awk 'BEGIN{FS="|"}{print $2}' <<< $LINE)
  echo "Running $LINE"
  /scratch2/jsybran/validation_tmp/runQuery.py -a -c $TMP_DIR -d $DATA -n 100 -e 1.4 -s -v $WORD1 $WORD2
}


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

