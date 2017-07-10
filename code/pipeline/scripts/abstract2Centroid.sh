#!/bin/bash
#PBS -N a2c
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/a2c.out
#PBS -e /home/jsybran/jobLogs/a2c.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

module load gcc

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

DATA=/zfs/safrolab/users/jsybran/moliere/data/hydrologySubset
DICT_FILE=$DATA/../fastText/canon.vec
ABS_FILE=$DATA/processedText/abstracts.txt
CEN_FILE=$DATA/fastText/centroids.data

makeCentroids -d $DICT_FILE -a $ABS_FILE -c $CEN_FILE


