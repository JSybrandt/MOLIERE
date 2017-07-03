#!/bin/bash
#PBS -N terms2knn
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/terms2knn.out
#PBS -e /home/jsybran/jobLogs/terms2knn.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

#usage: ./runFlann --inFile=string --graphFile=string --labelFile=string [options] ...
#options:
#  -d, --inFile              input vector file (string)
#  -g, --graphFile           output graph file (string)
#  -l, --labelFile           output label file (string)
#  -v, --verbose             outputs debug information
#  -n, --normalize           performs 0-1 scaling on edge weights
#  -e, --noSelfEdges         Removes self edges with distance 0
#  -u, --noDuplicateEdges    Removes only returns edges where i < j
#  -i, --ignoreFirstLine     Ignores the first line of the vector file
#  -s, --vecSize             dimensionality of vectors (unsigned int [=500])
#  -k, --numNN               expected num nearest neighbors (unsigned int [=10])
#  -?, --help                print this message

module load gcc
export C_INCLUDE_PATH=$C_INCLUDE_PATH:~/lib/include/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/lib

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

DATA=/zfs/safrolab/users/jsybran/moliere/data/hydrologySubset
CENTROID2VEC_FILE=$DATA/fastText/canon.vec
GRAPH_FILE=$DATA/network/terms.edges
LABEL_FILE=$DATA/network/terms.labels

runFlann -d $CENTROID2VEC_FILE -g $GRAPH_FILE -l $LABEL_FILE -neui -k 100

