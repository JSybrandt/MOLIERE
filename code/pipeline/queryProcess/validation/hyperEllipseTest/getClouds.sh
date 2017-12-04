#!/bin/bash
#PBS -N valPaths
#PBS -l select=1:ncpus=20:mem=120gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/valPaths.out
#PBS -e /home/jsybran/jobLogs/valPaths.err
#PBS -J 0-50
# the above is a default PBS header


echo $PBS_ARRAY_ID

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

if [ -z "$PBS_ARRAY_INDEX" ]; then
  echo "Must be submitted as array job"
  exit 1
fi

PBS_NUM_MACHINES=50

# add project tools to path
PATH=$PATH:$PROJ_HOME/code/components/links


# network info expected to be here
NET=$PROJ_HOME/data/yearlySubsets/2010/network
VECS=$PROJ_HOME/data/yearlySubsets/2010/fastText
TEXT=$PROJ_HOME/data/yearlySubsets/2010/processedText

ABSTRACTS=$TEXT/abstracts.txt
#ABSTRACTS=$TEXT/debug_abstracts.txt

NGRAM_VEC=$VECS/canon.vec
PMID_VEC=$VECS/centroids.data
PMID_VEC=$VECS/pmid_parts
# PMID_VEC=$VECS/debug_parts
UMLS_VEC=$VECS/umls.data

ELIPSE=1.4

OUT_DIR=$PROJ_HOME/results/validation/2010/hyperEllipseClouds
mkdir -p $OUT_DIR

QUERY_DIR=$PROJ_HOME/data/yearlySubsets/2010/validationData/parts

NUM_QUERIES=$(ls $QUERY_DIR | wc -l | awk '{print $1}')

Q_PER_MACHINE=$(($NUM_QUERIES / $PBS_NUM_MACHINES))

Q_START_NUM=$(($PBS_ARRAY_INDEX * $Q_PER_MACHINE))
Q_END_NUM=$(($Q_START_NUM + $Q_PER_MACHINE))

if [ $PBS_ARRAY_INDEX -eq $(($PBS_NUM_MACHINES - 1)) ]; then
  $Q_END_NUM=$NUM_QUERIES
fi

for((i = $Q_START_NUM; i < $Q_END_NUM; i++)){
  QUERY_FILE=$( ls $QUERY_DIR | sed -n "$(($i+1))"'p' )

  echo $QUERY_DIR/$QUERY_FILE
  findBatchBOWfromEllipse -q $QUERY_DIR/$QUERY_FILE\
    -o $OUT_DIR -V $NGRAM_VEC -P $PMID_VEC -U $UMLS_VEC -a $ABSTRACTS -v

}

#usage: ./findBatchBOWfromEllipse --queryFile=string --outDir=string --ngramVectors=string --pmidCentroids=string --umlsCentroids=string --abstractFile=string [options] ... 
#options:
  #-q, --queryFile        File containing pairs of labels. (string)
  #-o, --outDir           Directory to place results (string)
  #-V, --ngramVectors     File contanining text vectors for ngrams (string)
  #-P, --pmidCentroids    File containing text vectors for PMIDs (string)
  #-U, --umlsCentroids    File containing text vectors for UMLS terms (string)
  #-a, --abstractFile     File of PMID [abstract content] (string)
  #-e, --ellipseConst     Constant alpha where distL2(A,B)*\alpha = 2a (float [=1.4])
  #-m, --maxResult        Maximum number of documents to return (unsigned int [=20000])
  #-v, --verbose          outputs debug information
  #-?, --help             print this message

