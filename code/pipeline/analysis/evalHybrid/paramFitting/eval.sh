#!/bin/bash
#PBS -l select=1:ncpus=24:mem=100gb,walltime=24:00:00

module load gcc gnu-parallel openmpi

if [ -z "$OP_PARAM" ]; then
  echo "MUST BE SUBMITTED WITH qsub ... -v OP_PARAM='...'"
  exit 1
fi
if [ -z "$OUT_REAL" ]; then
  echo "MUST BE SUBMITTED WITH qsub ... -v OUT_REAL='...'"
  exit 1
fi
if [ -z "$OUT_FAKE" ]; then
  echo "MUST BE SUBMITTED WITH qsub ... -v OUT_FAKE='...'"
  exit 1
fi
if [ -z "$DONE" ]; then
  echo "MUST BE SUBMITTED WITH qsub ... -v DONE='...'"
  exit 1
fi
if [ -z "$SEND"]; then
  echo "MUST BE SUBMITTED WITH qsub ... -v SEND='...'"
  exit 1
fi


export OMP_NUM_THREADS=1
export OP_PARAM=$OP_PARAM
export OUT_REAL=$OUT_REAL
export OUT_FAKE=$OUT_FAKE
export DONE=$DONE

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

export CUID_VEC=$PROJ_HOME/data/yearlySubsets/2010/fastText/umls.data
export NGRAM_VEC=$PROJ_HOME/data/yearlySubsets/2010/fastText/canon.vec

evTopic(){
  f=$1
  if [ -f "$TOPIC_DIR/$f" ]; then
    SOURCE=$(awk 'BEGIN{FS="---"}{print $1}' <<< $f)
    TARGET=$(awk 'BEGIN{FS="---"}{print $2}' <<< $f)
    echo "evalHybrid -m $TOPIC_DIR/$f -n $NGRAM_VEC -c $CUID_VEC -s $SOURCE -t $TARGET -P \"$OP_PARAM\""
    evalHybrid -m $TOPIC_DIR/$f \
                       -n $NGRAM_VEC \
                       -c $CUID_VEC \
                       -s $SOURCE \
                       -t $TARGET \
                       -P "$OP_PARAM" \
                       >> $OUT
  fi
}

export -f evTopic

########################
# EVALUATE REAL TOPICS #
########################
export TOPIC_DIR=$PROJ_HOME/results/validation/2010/VIEW_REAL_SUBSET
export OUT=$OUT_REAL
parallel -j 24 evTopic {} ::: $(ls -f $TOPIC_DIR)

########################
# EVALUATE FAKE TOPICS #
########################
export TOPIC_DIR=$PROJ_HOME/results/validation/2010/VIEW_FAKE_SUBSET
export OUT=$OUT_FAKE
parallel -j 24 evTopic {} ::: $(ls -f $TOPIC_DIR)

./calcRoc.py $OUT_REAL $OUT_FAKE > $DONE

scp $DONE $SEND
