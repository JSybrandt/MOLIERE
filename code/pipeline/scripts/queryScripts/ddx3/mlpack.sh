#!/bin/bash
#PBS -N ddx3mlpack5
#PBS -l select=1:ncpus=16:mem=100gb,walltime=72:00:00
#PBS -J 1-50
#PBS -o /home/jsybran/jobLogs/ddx3mlpack5.out
#PBS -e /home/jsybran/jobLogs/ddx3mlpack5.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

for K in 2 3 4; do
DATA=/scratch2/jsybran/ddx3
MAT_FILE=$DATA/ddx3SimMat.pgm
OUT_DIR=$DATA/nmfRes/k-$K

mkdir -p $OUT_DIR

TRIAL_PER_NODE=1
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

  mlpack_nmf -i $MAT_FILE -r $K -u $ALG -W $W_FILE -H $H_FILE -e 1e-08
done
done
