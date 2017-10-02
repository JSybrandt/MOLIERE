#!/bin/bash
#PBS -N clouds2BOWF
#PBS -l select=1:ncpus=24:mem=100gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/clouds2BOWF.out
#PBS -e /home/jsybran/jobLogs/clouds2BOWF.err
#PBS -M jsybran@clemson.edu
#PBS -m ea
# the above is a default PBS header

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
# intermediary results
RES=$PROJ_HOME/results/validation/2010

CLOUD_FILE=$RES/allClouds.fake.predAug.txt
OUT_DIR=$RES/DATA_FAKE_PRED
mkdir -p $OUT_DIR

 # AB_FILE=$DATA/processedText/filtered_abstracts.predicateAugmented.txt
AB_FILE=$PROJ_HOME/data/processedText/filtered_abstracts.predicateAugmented.txt
LABELS=$DATA/network/final.labels

dijk2Data.py -l $LABELS -p $CLOUD_FILE -o $OUT_DIR -a $AB_FILE

#usage: dijk2Data.py [-h] [-l LABELPATH] [-p DIJKPATH] [-o OUTDIRPATH]
#                    [-a ABSTRACTPATH] [-v]

