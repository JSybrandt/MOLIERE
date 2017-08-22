#!/bin/bash
#PBS -N calNetCent
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/calNetCent.out
#PBS -e /home/jsybran/jobLogs/calNetCent.err
#PBS -M jsybran@clemson.edu
#PBS -m ea


# usage: calcCentrality.py [-h] [-g GRAPHPATH] [-o OUTPATH] [-s SAMPLES] [-v]
#
# optional arguments:
#   -h, --help            show this help message and exit
#   -g GRAPHPATH, --graph GRAPHPATH
#                         file path of the graph edge file
#   -o OUTPATH, --out OUTPATH
#                         dir path of result files
#   -s SAMPLES, --samples SAMPLES
#                         dir path of result files
#   -v, --verbose         print debug info

module load gcc
module load python/3.4

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

DATA=/scratch2/jsybran/moliere
GRAPH_FILE=$DATA/final.edges
SAMPLES=1000
CENTRALITY_FILE=$DATA/centrality.data

calcCentrality.py -g $GRAPH_FILE -o $CENTRALITY_FILE -s $SAMPLES

