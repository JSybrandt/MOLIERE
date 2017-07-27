#!/bin/bash
#PBS -N finialize
#PBS -l select=1:ncpus=16:mem=100gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/finalize.out
#PBS -e /home/jsybran/jobLogs/finalize.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

K=3
mkdir -p /scratch2/jsybran/moliere/nmfRes/k-$K-res/
pushd /zfs/safrolab/users/jsybran/moliere/code/netBuildTools/hypothesisClustering

./mlpack_finalize.jl -d /scratch2/jsybran/moliere/nmfRes/k-$K -W /scratch2/jsybran/moliere/nmfRes/k-$K-res/W.pgm -H /scratch2/jsybran/moliere/nmfRes/k-$K-res/H.pgm -X /scratch2/jsybran/moliere/hivAsscMat.pgm -k $K -v -C /scratch2/jsybran/moliere/nmfRes/k-$K-res/clusterRes.data -l /scratch2/jsybran/moliere/hivAsscHypoNames.txt

