#!/bin/bash
#PBS -N splitByFactor
#PBS -l select=1:ncpus=16:mem=100gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/splitByFactor.out
#PBS -e /home/jsybran/jobLogs/splitByFactor.err
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

mkdir -p $DATA/clusters

C_DATA=$DATA/factorResults/k-$K-res/clusterRes.data

for ((i=1; i<=$K; i++)); do
  CLUSTER_DIR=$DATA/clusters/cluster$i
  mkdir -p $CLUSTER_DIR
  ./createSubsetData.jl -c $C_DATA -m $CLUSTER_DIR/matrix.pgm -M $DATA/matrix.pgm -k $i -l $CLUSTER_DIR/hypoNames.txt -v
done

