#!/bin/bash
#PBS -N finialize
#PBS -l select=1:ncpus=16:mem=100gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/finalize.out
#PBS -e /home/jsybran/jobLogs/finalize.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

K=2
pushd /zfs/safrolab/users/jsybran/moliere/code/netBuildTools/hypothesisClustering
ORIG_DIR=/scratch2/jsybran/moliere/nmfRes/k-3-res
DATA_DIR=/scratch2/jsybran/moliere/nmfRes/k-3-res/subcluster1/k-$K
RES_DIR=/scratch2/jsybran/moliere/nmfRes/k-3-res/subcluster1/k-$K-res

mkdir -p $RES_DIR


./mlpack_finalize.jl -d $DATA_DIR -W $RES_DIR/W.pgm -H $RES_DIR/H.pgm -X $ORIG_DIR/cluster1.pgm -k $K -v -C $RES_DIR/clusterRes.data -l $ORIG_DIR/cluster1HypoNames.txt -v


