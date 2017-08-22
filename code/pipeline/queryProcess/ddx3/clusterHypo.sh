#!/bin/bash
#PBS -N ddx3cluster
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/ddx3cluster.out
#PBS -e /home/jsybran/jobLogs/ddx3cluster.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

# usage: hypoClustering -d DIJKSTRA-FILE -l LABEL-FILE
#                       [-r READ-TMP-FILE] [-w WRITE-TMP-FILE]
#                       [-f WRITE-RESULT-FILE] [-s START-K] [-e END-K]
#                       [-v] [-h]
#
# optional arguments:
#
#   -d, --dijkstra-file DIJKSTRA-FILE
#                         Path to the dijkstra + cloud file.
#   -l, --label-file LABEL-FILE
#                         Path to the graph label file for readability.
#   -r, --read-tmp-file READ-TMP-FILE
#                         Path to existing matrix A
#   -w, --write-tmp-file WRITE-TMP-FILE
#                         Path to write resulting matrix A
#   -f, --write-result-file WRITE-RESULT-FILE
#                         Path to write resulting matrix factors and
#                         cluster data.
#   -s, --start-k START-K
#                         First k to start trying, must be less than
#                         --end-k (type: Int64, default: 1)
#   -e, --end-k END-K     Last k to start trying, must be greater than
#                         --start-k (type: Int64, default: 5)
#   -v, --verbose
#   -h, --help            show this help message and exit

module load gcc

export JULIA_NUM_THREADS=24


PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

DATA=/scratch2/jsybran/ddx3
TARGET_FILE=$DATA/targets.indices
GRAPH_FILE=$DATA/final.edges
LABEL_FILE=$DATA/final.labels
DIJK_FILE=$DATA/ddx3.dijkstra
TMP_FILE=$DATA/ddx3AsscMatrix.dat
CLUSTER_FILE=$DATA/clusterRes.dat

hypoClustering -d $DIJK_FILE -l $LABEL_FILE -w $TMP_FILE -s 20 -e 30 -f $CLUSTER_FILE
