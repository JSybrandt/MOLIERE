#!/bin/bash
#PBS -N arrPldaHiv
#PBS -l select=1:ncpus=8:mem=30gb,walltime=72:00:00
#PBS -J 1-100
#PBS -o /home/jsybran/jobLogs/pldaHIV.out
#PBS -e /home/jsybran/jobLogs/pldaHIV.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

module load gcc mpich2/1.4-eth

DATA=/scratch2/jsybran/moliere/
DATA_DIR=$DATA/HIV_DATA_FILES
MODEL_DIR=$DATA/HIV_MODEL_FILES
VIEW_DIR=$DATA/HIV_VIEW_FILES

NUM_FILES=$(ls -f $DATA_DIR | wc -l)
echo "There are $NUM_FILES files in the data dir."

i=$PBS_ARRAY_INDEX

while [ $i -lt $NUM_FILES ]; do
  fName=$(ls -f $DATA_DIR | sed -n "$i"'p')
  if [ $fName != '.' ] && [ $fName != '..' ]; then
    #echo "Processing $fName"
    # if model does not exist
    if [ ! -f "$MODEL_DIR/$fName"'_0' ]; then
      echo "Model for $fName does not exist"
      in=$DATA_DIR/$fName
      out=$MODEL_DIR/$fName
      mpiexec -n 8 mpi_lda --num_topics 100 \
                           --alpha 1 \
                           --beta 0.01 \
                           --training_data_file $in \
                           --model_file $out \
                           --total_iterations 100 \
                           --burn_in_iterations 50 \
                           > /dev/null 2>&1
      #echo "Created Model for $fName"
      view_model.py "$MODEL_DIR/$fName"'_0' $VIEW_DIR/$fName
      #echo "Created view"
    fi
  fi
  i=$(($i+100))
done
