#!/bin/bash
#PBS -N evalHIVL2
#PBS -l select=1:ncpus=16:mem=50gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/evalCuid.out
#PBS -e /home/jsybran/jobLogs/evalCuid.err
#PBS -M jsybran@clemson.edu
#PBS -m ea
#PBS -J 0-99
# the above is a default PBS header

module load gcc gnu-parallel


NUM_MACHINES=100

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

# data expected to be here
DATA=$PROJ_HOME/data
RES=$PROJ_HOME/results

TOPIC_DIR=$PROJ_HOME/results/hiv-assc-dim/VIEW
TOPIC_FILE_LIST=$PROJ_HOME/results/hiv-assc-dim/VIEW_FILES.txt

CUID_VEC=$PROJ_HOME/data/fastText/umls.data
NGRAM_VEC=$PROJ_HOME/data/fastText/canon.vec

#OUT=$RES/validation/2010/allEvaluation.cosine.txt
OUT_DIR=$RES/hiv-assc-dim/L2
mkdir -p $OUT_DIR
OUT=$OUT_DIR/$PBS_ARRAY_INDEX.part.ev
rm $OUT

NUM_FILES=$(wc -l $TOPIC_FILE_LIST | awk '{print $1}')

JOB_RANGE=$(($NUM_FILES / $NUM_MACHINES))

START=$(($PBS_ARRAY_INDEX * $JOB_RANGE))
END=$(($START + $JOB_RANGE))
if [ $PBS_ARRAY_INDEX == $(($NUM_MACHINES - 1)) ]; then
  END=$NUM_FILES
fi

for((i = $START; i < $END; i++)){
  LINE=$( sed -n "$(($i+1))"'p' $TOPIC_FILE_LIST)
  in=$TOPIC_DIR/$LINE
  SOURCE=$(awk 'BEGIN{FS="---"}{print $1}' <<< $LINE)
  TARGET=$(awk 'BEGIN{FS="---"}{print $2}' <<< $LINE)
  evalWithEmbeddings -m $in \
                     -n $NGRAM_VEC \
                     -c $CUID_VEC \
                     -s $SOURCE \
                     -t $TARGET \
                     -e \
                     >> $OUT
}
