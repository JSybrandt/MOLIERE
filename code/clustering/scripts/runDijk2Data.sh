#!/bin/bash

module load python
module load sqlite

hypo_name=$1
input_path="/scratch2/esadrfa/moliere/"$hypo_name
rm -rf $input_path"/out"
mkdir $input_path"/out"

#cp /zfs/safrolab/users/jsybran/moliere/data/network/final.labels .
#cp /zfs/safrolab/users/jsybran/moliere/data/processedText/abstracts.txt .

#./dijk2Data.py -l ./final.labels  -p ./dijk2Data -o hiv/out/ -a ./abstracts.txt #-v
#./dijk2Data.py -l ./final.labels  -p ./test -o hiv/out/ -a ./abstracts.txt -v

#the main one 
./dijk2Data.py -l $input_path"/final.labels" -p $input_path"/clustered_clouds.txt" -o $input_path"/out/" -a $input_path"/abstracts.txt"

#only for HIV at Oct 1, 2017
#./dijk2Data.py -l $input_path"/final.labels" -p $input_path"/hiv_clustered_clouds.txt_old" -o $input_path"/out/" -a $input_path"/abstracts.txt"

