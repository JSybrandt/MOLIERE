#!/bin/bash
#PBS -N eHL2R
#PBS -l select=1:ncpus=28:mem=100gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/eHL2R.out
#PBS -e /home/jsybran/jobLogs/eHL2R.err
#PBS -M jsybran@clemson.edu
#PBS -m ea
# the above is a default PBS header

module load gcc gnu-parallel openmpi

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

TOPIC_DIR=$PROJ_HOME/results/validation/2010/VIEW_REAL

CUID_VEC=$PROJ_HOME/data/yearlySubsets/2010/fastText/umls.data
NGRAM_VEC=$PROJ_HOME/data/yearlySubsets/2010/fastText/canon.vec

OUT=$RES/validation/2010/real.hybrid.withMin.ev

rm $OUT

evTopic(){
  f=$1
  if [ -f "$TOPIC_DIR/$f" ]; then
    SOURCE=$(awk 'BEGIN{FS="---"}{print $1}' <<< $f)
    TARGET=$(awk 'BEGIN{FS="---"}{print $2}' <<< $f)
    echo "evalHybrid -m $TOPIC_DIR/$f -n $NGRAM_VEC -c $CUID_VEC -s $SOURCE -t $TARGET -B"
    evalHybrid -m $TOPIC_DIR/$f \
                       -n $NGRAM_VEC \
                       -c $CUID_VEC \
                       -s $SOURCE \
                       -t $TARGET \
                       -M \
                       >> $OUT
                       #-P "0 1.5203 .29332 1.9804 .87312 2.2369 .38104 1.0092" \
  fi
}

export -f evTopic
export TOPIC_DIR=$TOPIC_DIR
export OUT=$OUT
export CUID_VEC=$CUID_VEC
export NGRAM_VEC=$NGRAM_VEC
parallel -j 7 evTopic {} ::: $(ls -f $TOPIC_DIR)
