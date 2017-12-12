#!/bin/bash
#PBS -N playPath
#PBS -l select=1:ncpus=16:mem=60gb,walltime=72:00:00

SOURCE_LBL="torcetrapib"
TARGET_LBL="high_blood_pressure"

NUM_TOPICS=100

module load gcc python openmpi mpich

# Place us in the working directory
if [ -z "$PBS_O_WORKDIR" ]; then
  echo "Running from $PWD"
else
  echo "Moving to $PBS_O_WORKDIR"
  cd $PBS_O_WORKDIR
fi

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

DATA_ROOT=$PROJ_HOME/data
PLDA_EXEC=$PROJ_HOME/tools/plda/mpi_lda

# add project tools to path
PATH=$PATH:$PROJ_HOME/code/components/links

ELIPSE=1.4

NET=$DATA_ROOT/network
EDGES=$NET/final.bin.edges
LABELS=$NET/final.labels

VECS=$DATA_ROOT/fastText
NGRAM_VEC=$VECS/canon.vec
PMID_VEC=$VECS/centroids.data
UMLS_VEC=$VECS/umls.data

ABSTRACTS=$DATA_ROOT/processedText/abstracts.txt

echo "Searching for $SOURCE_LBL and $TARGET_LBL in $LABELS"
T1=$(mktemp)
T2=$(mktemp)
grep -nwm1 $SOURCE_LBL $LABELS | awk 'BEGIN{FS=":"}{print $1-1}' > "$T1" &
grep -nwm1 $TARGET_LBL $LABELS | awk 'BEGIN{FS=":"}{print $1-1}' > "$T2" &
wait
SOURCE_IDX=$(cat $T1)
TARGET_IDX=$(cat $T2)
rm -f $T1 $T2

if [ -z "$SOURCE_IDX" ]; then
  echo "FAILED TO FIND $SOURCE_LBL"
  exit 1
fi

if [ -z "$TARGET_IDX" ]; then
  echo "FAILED TO FIND $TARGET_LBL"
  exit 1
fi


PATH_FILE=`mktemp`
CLOUD_FILE=`mktemp`
BOW_FILE=`mktemp`
MODEL_FILE=`mktemp`
VIEW_FILE="./$SOURCE_LBL---$TARGET_LBL"

findPath -g $EDGES \
         -l $LABELS \
         -s $SOURCE_IDX \
         -t $TARGET_IDX \
         -V $NGRAM_VEC \
         -U $UMLS_VEC \
         -e $ELIPSE \
         -o $PATH_FILE \
         -v

findCloud -g $EDGES \
          -p $PATH_FILE \
          -o $CLOUD_FILE \
          -l $LABELS \
          -v

cloud2Bag -c $CLOUD_FILE \
          -o $BOW_FILE \
          -l $LABELS \
          -a $ABSTRACTS \
          -v

mpiexec -n 16 $PLDA_EXEC --num_topics $NUM_TOPICS \
                         --alpha 1 \
                         --beta 0.01 \
                         --training_data_file $BOW_FILE \
                         --model_file $MODEL_FILE \
                         --total_iterations 500 \
                         --burn_in_iterations 50

view_model.py $MODEL_FILE $VIEW_FILE


rm -rf $PATH_FILE
rm -rf $CLOUD_FILE
rm -rf $BOW_FILE
rm -rf $MODEL_FILE
