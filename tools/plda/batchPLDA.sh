#!/bin/bash
#PBS -N PLDA
#PBS -l select=1:ncpus=8:mem=30gb,walltime=24:00:00
#PBS -J 0-49

TASK_NAME=fulltext

module load gcc mpich2/1.4-eth

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

if [ -z "$PBS_ARRAY_INDEX" ]; then
  echo "must be submitted as array job"
  exit 1
fi

# add project tools to path
PATH=$PATH:$PROJ_HOME/code/components/links

RES_DIR=$PROJ_HOME/results/$TASK_NAME
DATA_DIR=$RES_DIR/DATA
MODEL_DIR=$RES_DIR/MODEL
VIEW_DIR=$RES_DIR/VIEW
DATA_FILE=$RES/allData.txt

if [! -f $DATA_FILE ]; then
  echo "Need to run pldaPrep.sh to get the allData.txt file."
  exit 1
fi

mkdir -p $MODEL_DIR
mkdir -p $VIEW_DIR

NUM_MACHINES=50
NUM_TOPICS=100

NUM_FILES=$(wc -l $DATA_FILE | awk '{print $1}')
echo "NUM: $NUM_FILES"

JOB_RANGE=$(($NUM_FILES / $NUM_MACHINES))

START=$(($PBS_ARRAY_INDEX * $JOB_RANGE))
END=$(($START + $JOB_RANGE))
if [ $PBS_ARRAY_INDEX == $(($NUM_MACHINES - 1)) ]; then
  END=$NUM_FILES
fi

echo "This job is responsible for files $START - $END"

for((i = $START; i < $END; i++)){
  LINE=$( sed -n "$(($i+1))"'p' $DATA_FILE)
  if [ ! -f $VIEW_DIR/$LINE ]; then
    in=$DATA_DIR/$LINE
    out=$MODEL_DIR/$LINE
    mpiexec -n 8 mpi_lda --num_topics $NUM_TOPICS \
                         --alpha 1 \
                         --beta 0.01 \
                         --training_data_file $in \
                         --model_file $out \
                         --total_iterations 500 \
                         --burn_in_iterations 50 \
                         > /dev/null 2>&1
    view_model.py "$MODEL_DIR/$LINE"'_0' $VIEW_DIR/$LINE
  fi
}
