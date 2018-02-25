#!/bin/bash
#PBS -N runFakeREDO
#PBS -l select=1:ncpus=24:mem=100gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/runFake.out
#PBS -e /home/jsybran/jobLogs/runFake.err
#PBS -M jsybran@clemson.edu
#PBS -m ea
#PBS -J 0-99
# the above is a default PBS header

module load gcc python mpich

# VALIDATION_FILE=$PROJ_HOME/data/validationData/redo/fake.txt
VALIDATION_FILE=/scratch2/jsybran/validation_tmp/redo/filtered_fake.txt
export VALIDATION_FILE

NUM_FILES=$(wc -l $VALIDATION_FILE | awk '{print $1}')

function run(){

  PROJ_HOME=$(pwd | grep -o .*moliere)
  PATH=$PATH:$PROJ_HOME/code/components/links
  DATA=$PROJ_HOME/data/yearlySubsets/2010

  TMP_DIR=/scratch2/jsybran/validation_tmp/redo/fake
  LINE=$( sed -n "$1p" $VALIDATION_FILE)
  WORD1=$(awk 'BEGIN{FS="|"}{print $1}' <<< $LINE)
  WORD2=$(awk 'BEGIN{FS="|"}{print $2}' <<< $LINE)
  /scratch2/jsybran/validation_tmp/runQuery.py -a -c $TMP_DIR -d $DATA -n 100 -e 1.4 $WORD1 $WORD2
}

export -f run

parallel --env run --env VALIDATION_FILE --workdir $PWD --progress --sshloginfile $PBS_NODEFILE "run {}" ::: $(seq 1 $NUM_FILES)


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

