#!/bin/bash
#PBS -N connectUMLS
#PBS -l select=1:ncpus=24:mem=100gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/connectUMLS.out
#PBS -e /home/jsybran/jobLogs/connectUMLS.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

# usage: ./connectUMLS --umlsDir=string --termList=string --outputFile=string [options] ...
# options:
#   -d, --umlsDir       input directory of umls (string)
#   -t, --termList      input list of terms (string)
#   -o, --outputFile    output file edge list (string)
#   -w, --weight
#   -v, --verbose       outputs debug information
#   -?, --help          print this message

module load gcc

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

DATA=/zfs/safrolab/users/jsybran/moliere/data/yearlySubsets/2010
UMLS_DIR=/zfs/safrolab/users/jsybran/moliere/data/umls/2009AB
TERMS_FILE=$DATA/network/terms.labels
OUT_FILE=$DATA/network/keyword2terms.edges

connectUMLS -d $UMLS_DIR -t $TERMS_FILE -o $OUT_FILE -v
