#!/bin/bash
#PBS -N runReal
#PBS -l select=1:ncpus=16:mem=60gb,walltime=72:00:00
#PBS -o /home/jsybran/jobLogs/runReal.out
#PBS -e /home/jsybran/jobLogs/runReal.err
#PBS -M jsybran@clemson.edu
#PBS -m ea
#PBS -J 0-49
# the above is a default PBS header

module load gcc python mpich gnu-parallel

cd $PBS_O_WORKDIR
PROJ_HOME=$(pwd | grep -o .*moliere)
export PROJ_HOME

# VALIDATION_FILE=$PROJ_HOME/data/validationData/onlyPairs/fake.pairs.txt
VALIDATION_FILE=/scratch2/jsybran/validation_tmp/finished.real.paths
export VALIDATION_FILE

NUM_FILES=$(wc -l $VALIDATION_FILE | awk '{print $1}')

DATA=/scratch2/jsybran/moliere/data/yearlySubsets/2010
export DATA

#TMP_DATA=/local_scratch/moliere
#export TMP_DATA

#echo "COPYING DATA"
#parallel --env DATA \
         #--env TMP_DATA \
         #-j 1\
         #--sshloginfile $PBS_NODEFILE \
         #"mkdir -p $TMP_DATA; cp -r $DATA/* $TMP_DATA" \
         #<<< " "
#echo "DONE"

function run(){
  module load gcc python mpich gnu-parallel

  LINE_NUM=$1
  PATH=$PATH:$PROJ_HOME/code/components/links
  # DATA=$PROJ_HOME/data/yearlySubsets/2010
  TMP_DIR=/scratch2/jsybran/validation_tmp/real

  LINE=$( sed -n "$LINE_NUM p" $VALIDATION_FILE)
  WORD1=$(awk '{print $1}' <<< $LINE)
  WORD2=$(awk '{print $2}' <<< $LINE)
  echo "$WORD1 $WORD2"
  export OMP_NUM_THREADS=8
  /scratch2/jsybran/validation_tmp/runQuery.py -a -c $TMP_DIR -d $DATA -n 100 -e 1.4 -s $WORD1 $WORD2
}
export -f run

NUM_MACHINES=50
JOB_RANGE=$(($NUM_FILES / $NUM_MACHINES))
JOB_START=$(($JOB_RANGE * $PBS_ARRAY_INDEX))
JOB_END=$(($JOB_START + $JOB_RANGE))
if [ $PBS_ARRAY_INDEX -eq $(($NUM_MACHINES - 1)) ]; then
  echo "LAST JOB"
  JOB_END=$NUM_FILES
fi

for((i=$JOB_START; i < $JOB_END; i=$i+1)){
  echo $i
  run $(($i+1))
}

##shuffle $VALIDATION_FILE
## sometimes parallel shits the bed, so we are gonna loop
#parallel --env VALIDATION_FILE \
         #--env PROJ_HOME \
         #--env DATA \
         #--env run \
         #-j 2 \
         #"run {}" ::: $(seq 1 $NUM_FILES)

#         --workdir $PBS_O_WORKDIR \
#         --sshloginfile $PBS_NODEFILE \

#usage: runQuery.py [-h] [-c CACHE] [-d DATA_HOME] [-n NUM_TOPICS]
                   #[-e ELLIPSE_CONSTANT] [-m] [-r] [-s] [-v]
                   #wordA wordB

#positional arguments:
  #wordA
  #wordB

#optional arguments:
  #-h, --help            show this help message and exit
  #-c CACHE, --cache CACHE
                        #specifies where to store cached files.
  #-d DATA_HOME, --data_home DATA_HOME
                        #specifies an anternate data directory
  #-n NUM_TOPICS, --num_topics NUM_TOPICS
                        #specifies the number of topics to generate.
  #-e ELLIPSE_CONSTANT, --ellipse_constant ELLIPSE_CONSTANT
                        #size of ellipse optimization
  #-m, --move_here       move topic / analysis files to working dir
  #-r, --reconstruct     if set, do not reuse existing cached files.
  #-s, --skip_sanitize   if set, do not check for input in labels.
  #-v, --verbose         if set, run pipeline with verbose flags.

