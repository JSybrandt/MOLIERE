#!/bin/bash
#PBS -N tfidf
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/tfidf.out
#PBS -e /home/jsybran/jobLogs/tfidf.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

#  usage: ./tfidf --docFile=string --termFile=string --outFile=string [options] ...
#  options:
#    -d, --docFile      input doc file (1 doc per line, name is first word) (string)
#    -t, --termFile     input term file (1 term per line) (string)
#    -o, --outFile      output edge file (string)
#    -v, --verbose      outputs debug information
#    -n, --normalize    performs 0-1 scaling on edge weights
#    -i, --inverse      calculates 1/tfidf for the purposes of shorestest path queries.
#    -?, --help         print this message

module load gcc

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

DATA=/zfs/safrolab/users/jsybran/moliere/data/hydrologySubset
ABSTRACT_FILE=$DATA/processedText/hydrologyDocuments.txt
KEYWORD_FILE=$DATA/processedText/termList.txt
EDGE_FILE=$DATA/network/tfidf.edges

tfidf -d $ABSTRACT_FILE -t $KEYWORD_FILE -o $EDGE_FILE -ni
