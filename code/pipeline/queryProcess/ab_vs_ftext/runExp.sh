#!/bin/bash
#PBS -N run
#PBS -l select=1:ncpus=64:mem=1900gb,walltime=72:00:00
#PBS -q bigmem
#PBS -m ea
#PBS -o /scratch2/jsybran/jobLogs/
#PBS -e /scratch2/jsybran/jobLogs/
# the above is a default PBS header

module load gcc python mpich

PBS_ARRAY_INDEX=0

if [ -z "$RF" ]; then
  echo "Did not specify 'real' or 'fake'"
  exit 1
fi

if [ -z "$EXP" ]; then
  echo "Did not specify 'abstract', 'hybrid', or 'fulltext'"
  exit 1
fi

cd $PBS_O_WORKDIR
echo $PWD

PAIR_FILE=./2015_pairs/$RF.dist.sample.pairs.txt
export PAIR_FILE

NUM_FILES=$(wc -l $PAIR_FILE | awk '{print $1}')
echo $NUM_FILES

DATA=/scratch2/$USER/$EXP-network
export DATA

CACHE=/scratch2/$USER/new_fake_cache/$EXP/$RF
export CACHE

mkdir -p $CACHE


function run(){
  LINE_NUM=$1
  LINE=$( sed -n "$LINE_NUM p" $PAIR_FILE)
  WORD1=$(awk 'BEGIN{FS="|"}{print $1}' <<< $LINE)
  WORD2=$(awk 'BEGIN{FS="|"}{print $2}' <<< $LINE)
  echo $WORD1 $WORD2
  ./moliere_deployed/runQuery.py \
    -c $CACHE \
    -d $DATA \
    -n 20 \
    -e 1.4 \
    -s \
    $WORD1 $WORD2
}
export -f run

NUM_MACHINES=1
JOB_RANGE=$(($NUM_FILES / $NUM_MACHINES))
JOB_START=$(($JOB_RANGE * $PBS_ARRAY_INDEX))
JOB_END=$(($JOB_START + $JOB_RANGE))
if [ $PBS_ARRAY_INDEX -eq $(($NUM_MACHINES - 1)) ]; then
  JOB_END=$NUM_FILES
fi

for((i=$JOB_START; i < $JOB_END; i=$i+1)){
  run $(($i+1))
}
