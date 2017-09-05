#!/bin/bash
#PBS -N valRunDij
#PBS -l select=1:ncpus=24:mem=100gb,walltime=72:00:00
#PBS -J 1-35
#PBS -o /home/jsybran/jobLogs/valRunDij.out
#PBS -e /home/jsybran/jobLogs/valRunDij.err
#PBS -M jsybran@clemson.edu
#PBS -m ea
# the above is a default PBS header

module load gcc openmpi

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

JOB_ARR_SIZE=35
if [ -z "$PBS_ARRAYID" ]; then
  JOB_ARR_SIZE=1
  PBS_ARRAYID=1
  echo "WARNING: This is not being run as an array job."
fi

# add project tools to path
PATH=$PATH:$PROJ_HOME/code/components/links

# data expected to be here
DATA=$PROJ_HOME/data/yearlySubsets/2010
# network info expected to be here
NET=$DATA/network
RES=$DATA/results/validation
mkdir -p $RES

GRAPH=$NET/final.edges
LABELS=$NET/final.labels
SOURCE_LIST=$DATA/database/sourceList.txt
TARGET_DIR=$DATA/database/validationTargets

DIJK_DIR=$RES/dijkstra
mkdir -p $DIJK_DIR


TOTAL_RUNS=$(wc -l $SOURCE_LIST | awk '{print $1}')

echo "Hypo file contains $TOTAL_RUNS lines"

START=$(bc <<< "($TOTAL_RUNS / $JOB_ARR_SIZE) * ($PBS_ARRAYID - 1)")
END=$(bc <<< "($TOTAL_RUNS / $JOB_ARR_SIZE) * ($PBS_ARRAYID)")

if [ $JOB_ARR_SIZE -eq $PBS_ARRAYID ]; then
  END=$TOTAL_RUNS
fi

echo "Running from $START to $END"

for ((i=$START; i<$END; i++)) {
  IDX_A=$(sed -n "$((i+1))p" $SOURCE_LIST)
  OUT="$DIJK_DIR/$IDX_A.validation.dijkstra"

  runDijkstra -g $GRAPH \
              -s $IDX_A \
              -T $TARGET_DIR/$IDX_A \
              -o $OUT   \
              -l $LABELS
}


#  usage: ./runDijkstra --graphFile=string --sourceIdx=unsigned int --outputFile=string [options] ...
#  options:
#    -g, --graphFile          input graph file (string)
#    -s, --sourceIdx          id representing the source (unsigned int)
#    -t, --targetIdx          intended target (unsigned int [=4294967295])
#    -T, --intendedTargets    intended targets (string [=])
#    -o, --outputFile         Output paths and neighborhoods (string)
#    -v, --verbose            outputs debug information
#    -n, --neighSize          number of nearby abstracts to include (unsigned int [=1000])
#    -a, --numAbstracts       number of abstracts in the network (unsigned int [=0])
#    -l, --labelFile          Label file accompanying the edges file. Used to count PMIDS (string [=])
#    -b, --abstractOffset     the index of the first abstract in the label file. (unsigned int [=0])
#    -N, --cloudSetN          abstract cloud param: number of new abstracts adjacent to those on path. (unsigned int [=2000])
#    -C, --cloudSetC          abstract cloud param: number of new abstracts from keyword overlap (unsigned int [=500])
#    -K, --cloudSetK          abstract cloud param: number of new abstracts from keywords (unsigned int [=500])
#    -?, --help               print this message


