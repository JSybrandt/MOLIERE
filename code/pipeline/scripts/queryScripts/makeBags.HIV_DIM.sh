#!/bin/bash
#PBS -N mkBgsHIV
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/mkBgsHIV.out
#PBS -e /home/jsybran/jobLogs/mkBgsHIV.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

# usage: dijk2Data [-h] [-l LABELPATH] [-p DIJKPATH] [-o OUTDIRPATH]
#                  [-d DATABASEPATH] [-v]
#
# optional arguments:
#   -h, --help            show this help message and exit
#   -l LABELPATH, --labelPath LABELPATH
#                         file path of the graph label file
#   -p DIJKPATH, --dijkstraResPath DIJKPATH
#                         file path of the dijkstra results
#   -o OUTDIRPATH, --outDir OUTDIRPATH
#                         dir path of result files
#   -d DATABASEPATH, --database DATABASEPATH
#                         file path of the sqlite database
#   -v, --verbose         print debug info

module load gcc
module load sqlite

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

DATA=/zfs/safrolab/users/jsybran/moliere/data
LABEL_FILE=$DATA/network/final.labels
SOURCE_NODE=25228039 #:hiv_associate_dementia
DIJK_FILE=$SCRATCH/hiv_associate_dementia.dijkstra
AB_FILE=$DATA/processedText/abstracts.txt
OUT_DIR=$SCRATCH/HIV_DATA_FILES

mkdir $OUT_DIR

dijk2Data -l $LABEL_FILE -p $DIJK_FILE -a $AB_FILE -o $OUT_DIR

