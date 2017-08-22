#!/bin/bash
#PBS -N genFactors
#PBS -l select=1:ncpus=16:mem=100gb,walltime=72:00:00
#PBS -J 1-10
#PBS -o /home/jsybran/jobLogs/genFactors.out
#PBS -e /home/jsybran/jobLogs/genFactors.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

X="FOO"
if [ -z ${K+X} ]; then
  echo "K is unset"
  exit 1
fi
if [ -z ${DATA+X} ]; then
  echo "DATA is unset"
  exit 1
fi

MAT_FILE=$DATA/matrix.pgm
OUT_DIR=$DATA/factorTests/k-$K

mkdir -p $OUT_DIR

TRIAL_PER_NODE=5
NODE_ID=$PBS_ARRAY_INDEX

if [ -z ${PBS_ARRAY_INDEX+x} ]; then
  PBS_ARRAY_INDEX=1
fi

# ensures we get different seeds
sleep $PBS_ARRAY_INDEX

ALG="als"

for TRIAL in $(seq 1 $TRIAL_PER_NODE); do
  JOB_ID="$K.$NODE_ID.$TRIAL"
  W_FILE="$OUT_DIR/W.$JOB_ID.pgm"
  H_FILE="$OUT_DIR/H.$JOB_ID.pgm"

  mlpack_nmf -i $MAT_FILE -r $K -u $ALG -W $W_FILE -H $H_FILE -e 1e-07 -v
done
