#!/bin/bash
#PBS -N batchHAD
#PBS -l select=1:ncpus=16:mem=60gb,walltime=72:00:00
#PBS -J 0-199
#PBS -o /dev/null
#PBS -e /dev/null


module load gcc mpich python

TARGET="hiv_associate_dementia"

# Identify the project home dir
if [ -z "$MOLIERE_HOME" ]; then
  echo "searching for moliere home directory"
  MOLIERE_HOME=$(pwd | grep -o .*moliere)
  if [ "$MOLIERE_HOME" = "" ]; then
    echo "Failed to find project home"
    exit 1
  else
    echo "Found $MOLIERE_HOME"
  fi
fi

if [ -z "$PBS_ARRAY_INDEX" ]; then
  echo "must be submitted as array job"
  exit 1
fi

# add project tools to path
PATH=$PATH:$MOLIERE_HOME/code/components/links

RES=$MOLIERE_HOME/results/hiv-assc-dim
cd $RES

CACHE=/scratch2/jsybran/cache
LOG=/scratch2/jsybran/log.txt

GENE_FILE=$MOLIERE_HOME/data/geneData/subsetGeneList

NUM_MACHINES=200

NUM_GENES=$(wc -l $GENE_FILE | awk '{print $1}')

JOB_RANGE=$(($NUM_GENES / $NUM_MACHINES))

START=$(($PBS_ARRAY_INDEX * $JOB_RANGE))
END=$(($START + $JOB_RANGE))
if [ $PBS_ARRAY_INDEX == $(($NUM_MACHINES - 1)) ]; then
  END=$NUM_GENES
fi


for((i = $START; i < $END; i++)){
  GENE=$( sed -n "$(($i+1))"'p' $GENE_FILE)
  TOPIC_FILE1="$GENE---$TARGET.100.eval"
  TOPIC_FILE2="$TARGET---$GENE.100.eval"
  if [ -f $TOPIC_FILE1 ] || [ -f $TOPIC_FILE2 ]; then
    echo "Skipping $GENE" >> $LOG
  else
    echo "BEGIN: $GENE" >> $LOG
    runQuery.py --cache $CACHE \
                --move_here \
                --skip_sanitize \
                $TARGET \
                $GENE \
                >> $LOG
  fi
}


#usage: runQuery.py [-h] [-c CACHE] [-d DATA_HOME] [-n NUM_TOPICS] [-m] [-r]
                   #[-v]
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
  #-m, --move_here       move topic / analysis files to working dir
  #-r, --reconstruct     if set, do not reuse existing cached files.
  #-v, --verbose         if set, run pipeline with verbose flags.

