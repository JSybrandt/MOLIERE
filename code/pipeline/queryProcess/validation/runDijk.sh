#!/bin/bash
#PBS -N valRunDij
#PBS -l select=1:ncpus=64:mem=1900gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/valRunDij.out
#PBS -e /home/jsybran/jobLogs/valRunDij.err
#PBS -M jsybran@clemson.edu
#PBS -m ea
# the above is a default PBS header

module load gcc python

# Place us in the working directory
if [ -z "$PBS_O_WORKDIR" ]; then
  echo "Must be submitted through PBS from home directory"
  exit 1
fi
cd $PBS_O_WORKDIR

# Identify the project home dir
if [ -z "$PROJ_HOME" ]; then
  echo "Searching for moliere home directory"
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

# network info expected to be here
NET=$DATA/network
RES=$DATA/results/validation
mkdir -p $RES

EDGES=$NET/final.edges
LABELS=$NET/final.labels
PROB_DESC=$DATA/database/validationPairs.txt
DIJK_PATH=$RES/validation.dijkstra


findPaths.py -e $EDGES \
             -l $LABELS \
             -p $PROB_DESC \
             > $DIJK_PATH
