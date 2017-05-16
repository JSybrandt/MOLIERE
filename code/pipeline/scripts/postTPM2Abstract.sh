#!/bin/bash
#PBS -N postTPM2Abstract
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/postTPM2Abstract.out
#PBS -e /home/jsybran/jobLogs/postTPM2Abstract.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

# usage: ./postTPM2Abstract --postTPMFile=string --pmidFile=string --outputFile=string [options] ...
# options:
#   -t, --postTPMFile      input directory of umls (string)
#   -p, --pmidFile         input directory of umls (string)
#   -o, --outputFile       output abstract file (string)
#   -d, --dummySentence    dummy phrase which we should throw out. (string [=quick_brown_fox])
#   -v, --verbose          outputs debug information
#   -?, --help             print this message

module load gcc

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

DATA=/zfs/safrolab/users/jsybran/moliere/data
TEXT=$DATA/processedText
CANON_POST_TPM=$TEXT/canon_post_topmine.txt
PMID=$TEXT/pmid.txt
OUT=$TEXT/abstracts.txt

postTPM2Abstract -t $CANON_POST_TPM -p $PMID -o $OUT
