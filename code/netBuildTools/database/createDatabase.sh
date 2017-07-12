#!/bin/bash
#PBS -N makeDBYear
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/makeDBYear.out
#PBS -e /home/jsybran/jobLogs/makeDBYear.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

pushd /zfs/safrolab/users/jsybran/moliere/code/netBuildTools/database

module load python/3.4
module load sqlite

# ./populateAbstractDB.py -d /zfs/safrolab/users/jsybran/moliere/data/database/abstracts.db -a /zfs/safrolab/users/jsybran/moliere/data/processedText/abstracts.txt -r

./fillDates.py -x /zfs/safrolab/users/jsybran/moliere/data/rawData -d /zfs/safrolab/users/jsybran/moliere/data/database/abstracts.db
