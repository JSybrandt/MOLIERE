#!/bin/bash
#PBS -N HIVOMPQ
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/HIVOMPQ.out
#PBS -e /home/jsybran/jobLogs/HIVOMPQ.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

# usage: ./runDijkstra --graphFile=string --sourceNode=string --outputFile=string [options] ... 
# options:
#   -g, --graphFile           input graph file (string)
#   -s, --sourceNode          id representing the source (string)
#   -t, --intendedTargets     intended targets. (string [=])
#   -o, --outputFile          Output paths and neighborhoods (string)
#   -v, --verbose             outputs debug information
#   -n, --neighSize           number of nearby abstracts to include (unsigned int [=1000])
#   -?, --help                print this message

module load gcc

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

DATA=/scratch2/jsybran/moliere
TARGET_FILE=$DATA/geneList.indices
GRAPH_FILE=$DATA/final.edges
SOURCE_NODE=24139454
OUT_FILE=$DATA/hiv_associate_dementia.dijkstra

runDijkstra -g $GRAPH_FILE -s $SOURCE_NODE -t $TARGET_FILE -n 10000 -o $OUT_FILE
