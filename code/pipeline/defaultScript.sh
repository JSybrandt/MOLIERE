#!/bin/bash
#PBS -N jobName
#PBS -l select=1:ncpus=24:mem=100gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/jobName.out
#PBS -e /home/jsybran/jobLogs/jobName.err
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
DATA=$PROJ_HOME/data
# network info expected to be here
NETWORK=$DATA/network
