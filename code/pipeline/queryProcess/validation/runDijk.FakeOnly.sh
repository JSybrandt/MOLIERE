#!/bin/bash
#PBS -N valFakePaths
#PBS -l select=1:ncpus=16:mem=60gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/valFakePaths.out
#PBS -e /home/jsybran/jobLogs/valFakePaths.err
#PBS -J 0-99
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

if [ -z "$PBS_ARRAY_INDEX" ]; then
  echo "Must be submitted as array job"
  exit 1
fi

PBS_NUM_MACHINES=100

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

ELIPSE=1.4

OUT_DIR=$PROJ_HOME/results/validation/2010/fakepaths
mkdir -p $OUT_DIR

QUERY_FILE=$PROJ_HOME/data/yearlySubsets/2010/database/validationSet.umls.fakeSubset.txt

NUM_QUERIES=$(wc -l $QUERY_FILE | awk '{print $1}')

Q_PER_MACHINE=$(($NUM_QUERIES / $PBS_NUM_MACHINES))

Q_START_NUM=$(($PBS_ARRAY_INDEX * $Q_PER_MACHINE))
Q_END_NUM=$(($Q_START_NUM + $Q_PER_MACHINE))

if [ $PBS_ARRAY_INDEX -eq $(($PBS_NUM_MACHINES - 1)) ]; then
  $Q_END_NUM=$NUM_QUERIES
fi

for((i = $Q_START_NUM; i < $Q_END_NUM; i++)){
  LINE=$( sed -n "$(($i+1))"'p' $QUERY_FILE)
  SOURCE_LBL=$(awk 'BEGIN{FS="|"}{print $1}' <<< $LINE)
  VERB_TOKEN=$(awk 'BEGIN{FS="|"}{print $2}' <<< $LINE)
  TARGET_LBL=$(awk 'BEGIN{FS="|"}{print $3}' <<< $LINE)
  YEAR=$(awk 'BEGIN{FS="|"}{print $4}' <<< $LINE)

  SOURCE_IDX=$( grep -nwm1 $SOURCE_LBL $LABELS | awk 'BEGIN{FS=":"}{print $1-1}')
  TARGET_IDX=$( grep -nwm1 $TARGET_LBL $LABELS | awk 'BEGIN{FS=":"}{print $1-1}')

  OUT="$OUT_DIR/$SOURCE_LBL-$VERB_TOKEN-$TARGET_LBL.path"

  if [ ! -f $OUT ]; then
    findPath -g $EDGES \
             -l $LABELS \
             -s $SOURCE_IDX \
             -t $TARGET_IDX \
             -V $NGRAM_VEC \
             -P $PMID_VEC \
             -U $UMLS_VEC \
             -e $ELIPSE \
             -o $OUT
  fi
}


#usage: ./findPath --graphFile=string --sourceIdx=unsigned int --outputFile=string --ngramVectors=string --pmidCentroids=string --umlsCentroids=string --labelFile=string --elipseConst=float [options] ...
#options:
  #-g, --graphFile        input graph file (string)
  #-s, --sourceIdx        id representing the source (unsigned int)
  #-t, --targetIdx        intended target (unsigned int [=4294967295])
  #-o, --outputFile       Output paths and neighborhoods (string)
  #-V, --ngramVectors     File contanining text vectors for ngrams (string)
  #-P, --pmidCentroids    File containing text vectors for PMIDs (string)
  #-U, --umlsCentroids    File containing text vectors for UMLS terms (string)
  #-l, --labelFile        Label file accompanying the edges file. (string)
  #-e, --elipseConst      Constant alpha where dist(A,B)*\alpha = 2a (float)
  #-v, --verbose          outputs debug information
  #-?, --help             print this message
