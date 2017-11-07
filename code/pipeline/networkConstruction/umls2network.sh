#!/bin/bash
#PBS -N umlsNetwork
#PBS -l select=1:ncpus=24:mem=100gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/umlsNetwork.out
#PBS -e /home/jsybran/jobLogs/umlsNetwork.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

# usage: ../tools/umlsNetwork --umlsDir=string --outputFile=string [options] ... 
# options:
#   -d, --umlsDir        input directory of umls (string)
#   -o, --outputFile     output file name, will result in .edges and .labels (string)
#   -v, --verbose        outputs debug information
#   -w, --edgeWeights    the maximal value of resulting edge weights (float [=1])
#   -?, --help           print this message

module load gcc

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

DATA=/zfs/safrolab/users/jsybran/moliere/data/yearlySubsets/2010
UMLS_DIR=/zfs/safrolab/users/jsybran/moliere/data/umls/2009AB
NET_FILE=$DATA/network/umls

umlsNetwork -d $UMLS_DIR -o $NET_FILE

