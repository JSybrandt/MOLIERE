#!/bin/bash
#PBS -N fasttext
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/fasttext.out
#PBS -e /home/jsybran/jobLogs/fasttext.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

module load gcc
module load openmpi

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

THREADS=24

DATA=/zfs/safrolab/users/jsybran/moliere/data
TEXT=$DATA/processedText
ABSTRACTS=$TEXT/abstracts.txt
OUTPUT=$DATA/fastText/canon

fasttext skipgram -minCount 10 -dim 500 -ws 8 -maxn 8 -thread $THREADS -input $ABSTRACTS -output $OUTPUT
