#!/bin/bash
#PBS -N pldaCOC
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/pldaCOC.out
#PBS -e /home/jsybran/jobLogs/pldaCOC.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

module load gnu-parallel
module load gcc
module load mpich2

DATA=/zfs/safrolab/users/jsybran/moliere/data
DATA_DIR=$SCRATCH/COC_DATA_FILES
MODEL_DIR=$SCRATCH/COC_MODEL_FILES
VIEW_DIR=$SCRATCH/COC_VIEW_FILES

mkdir $MODEL_DIR
mkdir $VIEW_DIR

parallel -j8 runPLDA1 {} $MODEL_DIR/{/}  ::: $DATA_DIR/*

parallel view_model.py {} $VIEW_DIR/{/} ::: $MODEL_DIR/*
