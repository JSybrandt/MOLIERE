#!/bin/bash
#PBS -N playPath
#PBS -l select=1:ncpus=16:mem=60gb,walltime=72:00:00

SOURCE_LBL="torcetrapib"
TARGET_LBL="high_blood_pressure"

INTERM_DATA_DIR="/scratch2/jsybran/cache"

NUM_TOPICS=100

module purge
module load gcc openmpi

# Place us in the working directory
if [ -z "$PBS_O_WORKDIR" ]; then
  echo "Running from $PWD"
else
  echo "Moving to $PBS_O_WORKDIR"
  cd $PBS_O_WORKDIR
fi

# Identify the project home dir
if [ -z "$PROJ_HOME" ]; then
  echo "Searching for moliere home directory"
  PROJ_HOME=$(pwd | grep -o .*moliere)
  if [ "$PROJ_HOME" = "" ]; then
    echo "Failed to find project home"
    exit 1
  else
    echo "Found $PROJ_HOME"
  fi
fi

DATA_ROOT=$PROJ_HOME/data
PLDA_EXEC=$PROJ_HOME/tools/plda/mpi_lda

# add project tools to path
PATH=$PATH:$PROJ_HOME/code/components/links

ELIPSE=1.4

NET=$DATA_ROOT/network
EDGES=$NET/final.bin.edges
LABELS=$NET/final.labels

VECS=$DATA_ROOT/fastText
NGRAM_VEC=$VECS/canon.vec
PMID_VEC=$VECS/centroids.data
UMLS_VEC=$VECS/umls.data

ABSTRACTS=$DATA_ROOT/processedText/abstracts.txt

recover_or_gen_file(){
  EXTENSION=$1

  FILE_PATTERN="$INTERM_DATA_DIR/$SOURCE_LBL---$TARGET_LBL.*.$EXTENSION"
  FILE_TEMPLATE="$INTERM_DATA_DIR/$SOURCE_LBL---$TARGET_LBL.XXXX.$EXTENSION"
  RET=""
  REUSE=0

  # if path file exists
  if [ -f "$FILE_PATTERN" ]; then
    RET=`ls "$FILE_PATTERN"`
    read -p "Use pre-existing $EXTENSION file? [y/n]" -n 1 -r
  else  # if no file, no reuse
    REPLY="n"
  fi

  # if not reusing file
  if [[ $REPLY =~ ^[nN]$ ]]; then
    RET=`mktemp $FILE_TEMPLATE`
  else
    REUSE=1
  fi
  echo "$RET $REUSE"
}

read PATH_FILE REUSE_PATH <<< $(recover_or_gen_file path)
read CLOUD_FILE REUSE_CLOUD <<< $(recover_or_gen_file cloud)
read BOW_FILE REUSE_BOW <<< $(recover_or_gen_file bow)
read MODEL_FILE REUSE_MODEL <<< $(recover_or_gen_file model)

VIEW_FILE="$SOURCE_LBL---$TARGET_LBL"
EVAL_FILE="$SOURCE_LBL---$TARGET_LBL.eval"

if [ $REUSE_PATH == 0 ]; then
  findPath -g $EDGES \
           -l $LABELS \
           -s $SOURCE_LBL \
           -t $TARGET_LBL \
           -V $NGRAM_VEC \
           -U $UMLS_VEC \
           -e $ELIPSE \
           -o $PATH_FILE \
           -v
fi

if [ $REUSE_CLOUD == 0 ]; then
  findCloud -g $EDGES \
            -p $PATH_FILE \
            -o $CLOUD_FILE \
            -l $LABELS \
            -v
fi

if [ $REUSE_BOW == 0 ]; then
  cloud2Bag -c $CLOUD_FILE \
            -o $BOW_FILE \
            -l $LABELS \
            -a $ABSTRACTS \
            -v
fi

module purge
module load gcc mpich python

if [ $REUSE_MODEL == 0 ]; then
  mpiexec -n 16 $PLDA_EXEC --num_topics $NUM_TOPICS \
                           --alpha 1 \
                           --beta 0.01 \
                           --training_data_file $BOW_FILE \
                           --model_file $MODEL_FILE \
                           --total_iterations 500 \
                           --burn_in_iterations 50
fi

view_model.py $MODEL_FILE > $VIEW_FILE

module purge
module load gcc openmpi
PARAMS="0 1.5203 .29332 1.9804 .87312 2.2369 .38104 1.0092"

evalHybrid -m $VIEW_FILE \
           -n $NGRAM_VEC \
           -c $CUID_VEC \
           -p $PMID_VEC \
           -s $SOURCE_LBL \
           -t $TARGET_LBL \
           -P $PARAMS \
           >  $EVAL_FILE


