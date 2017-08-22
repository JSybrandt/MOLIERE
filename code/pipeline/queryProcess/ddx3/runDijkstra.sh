#!/bin/bash
#PBS -N ddx3dijk
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/ddx3dijk.out
#PBS -e /home/jsybran/jobLogs/ddx3dijk.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

#  -g, --graphFile          input graph file (string)
#  -s, --sourceIdx          id representing the source (unsigned int)
#  -t, --intendedTargets    intended targets. (string [=])
#  -o, --outputFile         Output paths and neighborhoods (string)
#  -v, --verbose            outputs debug information
#  -n, --neighSize          number of nearby abstracts to include (unsigned int [=1000])
#  -a, --numAbstracts       number of abstracts in the network (unsigned int)
#  -b, --abstractOffset     the index of the first abstract in the label file. (unsigned int [=0])
#  -N, --cloudSetN          abstract cloud param: number of new abstracts adjacent to those on path. (unsigned int [=2000])
#  -C, --cloudSetC          abstract cloud param: number of new abstracts from keyword overlap (unsigned int [=500])
#  -K, --cloudSetK          abstract cloud param: number of new abstracts from keywords (unsigned int [=500])
#  -?, --help               print this message

module load gcc

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

DATA=/scratch2/jsybran/ddx3
TARGET_FILE=$DATA/targets.indices
GRAPH_FILE=$DATA/final.edges
LABEL_FILE=$DATA/final.labels
SOURCE_NODE=22451131 #ddx3
OUT_FILE=$DATA/ddx3.dijkstra

runDijkstra -g $GRAPH_FILE -s $SOURCE_NODE -t $TARGET_FILE -o $OUT_FILE -l $LABEL_FILE
