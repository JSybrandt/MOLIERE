#!/bin/bash
#PBS -N analyzeFactors
#PBS -l select=1:ncpus=16:mem=100gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/analyzeFactors.out
#PBS -e /home/jsybran/jobLogs/analyzeFactors.err
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

pushd /zfs/safrolab/users/jsybran/moliere/code/netBuildTools/hypothesisClustering

FACTOR_DIR=$DATA/factorTests/k-$K
RES_DIR=$DATA/factorResults/k-$K-res

mkdir -p $RES_DIR


./mlpack_finalize.jl -d $FACTOR_DIR -W $RES_DIR/W.pgm -H $RES_DIR/H.pgm -X $DATA/matrix.pgm -k $K -v -C $RES_DIR/clusterRes.data -l $DATA/hypoNames.txt -v


