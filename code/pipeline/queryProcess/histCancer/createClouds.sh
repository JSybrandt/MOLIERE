#!/bin/bash
#PBS -N histClouds
#PBS -l select=1:ncpus=24:mem=100gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/histClouds.out
#PBS -e /home/jsybran/jobLogs/histClouds.err
# the above is a default PBS header


echo $PBS_ARRAY_ID

module load gcc python openmpi

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

# if [ -z "$PBS_ARRAY_INDEX" ]; then
#   echo "Must be submitted as array job"
#   exit 1
# fi
#
# PBS_NUM_MACHINES=100

# add project tools to path
PATH=$PATH:$PROJ_HOME/code/components/links


# network info expected to be here
NET=$PROJ_HOME/data/yearlySubsets/2010/network
VECS=$PROJ_HOME/data/yearlySubsets/2010/fastText

EDGES=$NET/final.bin.edges
LABELS=$NET/final.labels

NGRAM_VEC=$VECS/canon.vec
PMID_VEC=$VECS/centroids.data
#PMID_VEC=$VECS/empty.data
#touch $PMID_VEC
UMLS_VEC=$VECS/umls.data


OUT=$PROJ_HOME/results/cancer2010/allClouds.txt

PATHS=$PROJ_HOME/results/cancer2010/allPaths.txt

paths2Dijk -g $EDGES -p $PATHS -o $OUT -l $LABELS

#usage: ./paths2Dijk --graphFile=string --pathFile=string --outputFile=string --labelFile=string [options] ... 
#options:
  #-g, --graphFile     input graph (BINARY) file (string)
  #-p, --pathFile      input path file (idx) (string)
  #-o, --outputFile    Output paths and neighborhoods (string)
  #-l, --labelFile     Label file accompanying the edges file. (string)
  #-N, --cloudSetN     abstract cloud param: number of new abstracts adjacent to those on path. (unsigned int [=2000])
  #-C, --cloudSetC     abstract cloud param: number of new abstracts from keyword overlap (unsigned int [=500])
  #-K, --cloudSetK     abstract cloud param: number of new abstracts from keywords (unsigned int [=500])
  #-v, --verbose       outputs debug information
  #-?, --help          print this message
