#!/bin/bash

module load python/3.4

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

DATA_DIR=/zfs/safrolab/users/jsybran/moliere/data/hydrologySubset

filterAbstractFile.py -d $LABHOME/moliere/data/database/abstracts.db -a $DATA_DIR/processedText/abstracts.txt -o $DATA_DIR/processedText/filtered_abstracts_2013.txt -y 2013
