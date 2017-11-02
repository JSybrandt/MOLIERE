#!/bin/bash
#PBS -N fasttext
#PBS -l select=1:ncpus=64:mem=1900gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/fasttext.out
#PBS -e /home/jsybran/jobLogs/fasttext.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

module load gcc
module load openmpi

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

THREADS=64

DATA=/zfs/safrolab/users/jsybran/moliere/data/yearlySubsets/2010
TEXT=$DATA/processedText
ABSTRACTS=$TEXT/abstracts.txt
OUTPUT=$DATA/fastText/canon.retrained

fasttext skipgram -minCount 10 -dim 500 -ws 8 -maxn 8 -thread $THREADS -input $ABSTRACTS -output $OUTPUT
