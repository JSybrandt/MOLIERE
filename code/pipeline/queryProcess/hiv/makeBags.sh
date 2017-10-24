#!/bin/bash
#PBS -N mkBgsHIV
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/mkBgsHIV.out
#PBS -e /home/jsybran/jobLogs/mkBgsHIV.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

module load gcc openmpi

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
# network info expected to be here
NETWORK=$DATA/network
# results go here
RES=$PROJ_HOME/results/hiv-assc-dim

OUT_DIR=$RES/DATA
mkdir -p $OUT_DIR
LABELS=$NETWORK/final.labels
DIJK_FILE=$RES/hiv_associate_dementia.dijkstra
FIXED_DIJK_FILE=$RES/hiv_associate_dementia.fixed.dijkstra

# convert dijk file
# awk 'NR % 2 {out=""; for(i=6; i<=NF-2; i++){out=out" "$i}; print out} NR % 2 == 0{print}' $DIJK_FILE > $FIXED_DIJK_FILE


AB_FILE=$DATA/processedText/filtered_abstracts.txt

dijk2Data.py -l $LABELS -p $FIXED_DIJK_FILE -a $AB_FILE -o $OUT_DIR
