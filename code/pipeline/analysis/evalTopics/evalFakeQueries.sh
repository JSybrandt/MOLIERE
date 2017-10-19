#!/bin/bash
#PBS -N evalFake
#PBS -l select=1:ncpus=24:mem=100gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/evalFake.out
#PBS -e /home/jsybran/jobLogs/evalFake.err
#PBS -M jsybran@clemson.edu
#PBS -m ea
# the above is a default PBS header

module load gcc gnu-parallel

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
RES=$PROJ_HOME/results

MRCONSO=$DATA/umls/2009.MRCONSO

TOPIC_DIR=$PROJ_HOME/results/validation/2010/VIEW_FAKE

OUT=$RES/validation/2010/allEvaluation.topic.fake.txt
VAL_DATA=$PROJ_HOME/data/validationData/validationSet.umls.txt

rm $OUT

evTopic(){
  f=$1
  if [ -f "$TOPIC_DIR/$f" ]; then
    SOURCE=$(awk 'BEGIN{FS="---"}{print $1}' <<< $f)
    TARGET=$(awk 'BEGIN{FS="---"}{print $2}' <<< $f)
    VERBS=$(grep -w $SOURCE $VAL_DATA | grep -w $TARGET | \
           awk 'BEGIN{FS="|"}{print tolower($2)}')
    echo $SOURCE $TARGET
    while read -r VERB; do
      evalPredicate -m $TOPIC_DIR/$f \
                    -M $MRCONSO \
                    -s $SOURCE \
                    -t $TARGET \
                    -b $VERB \
                    >> $OUT
    done <<< $VERBS
  fi
}

export -f evTopic
export TOPIC_DIR=$TOPIC_DIR
export MRCONSO=$MRCONSO
export OUT=$OUT
export VAL_DATA=$VAL_DATA
parallel evTopic ::: $(ls $TOPIC_DIR)
