#!/bin/bash
#PBS -N nmfk10
#PBS -l select=1:ncpus=16:mem=100gb,walltime=72:00:00
#PBS -J 1-10
#PBS -o /home/jsybran/jobLogs/nmfk10.out
#PBS -e /home/jsybran/jobLogs/nmfk10.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

K=10
DATA=/scratch2/jsybran/moliere
MAT_FILE=$DATA/hivAsscMat.pgm
OUT_DIR=$DATA/nmfRes/k-$K

mkdir -p $OUT_DIR

TRIAL_PER_NODE=5
NODE_ID=$PBS_ARRAY_INDEX

if [ -z ${PBS_ARRAY_INDEX+x} ]; then
  PBS_ARRAY_INDEX=1
fi

ALG="als"

for TRIAL in $(seq 1 $TRIAL_PER_NODE); do
  JOB_ID="$K.$NODE_ID.$TRIAL"
  W_FILE="$OUT_DIR/W.$JOB_ID.pgm"
  H_FILE="$OUT_DIR/H.$JOB_ID.pgm"

  mlpack_nmf -i $MAT_FILE -r $K -u $ALG -W $W_FILE -H $H_FILE -e 1e-08
done
