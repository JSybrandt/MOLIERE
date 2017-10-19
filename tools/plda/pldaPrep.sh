#!/bin/bash

TASK_NAME=$1

if [ -z "$TASK_NAME" ]; then
  echo "MUST SUBMIT WITH PROJECT NAME AS COMAND ARG 1"
  exit 1
fi

if [ ! -z "$2" ]; then
  echo "pldaPrep doesn not accept more than one argument."
  exit 1
fi

# Identify the project home dir
if [ -z "$PROJ_HOME" ]; then
  echo "searching for moliere home directory"
  PROJ_HOME=$(pwd | grep -o .*moliere)
  if [ "$PROJ_HOME" = "" ]; then
    echo "Failed to find project home"
    exit 1
  else
    echo "Found $PROJ_HOME"
  fi
fi

RES_DIR=$PROJ_HOME/results/$TASK_NAME
DATA_FILE=$RES_DIR/allData.txt
DATA_DIR=$RES_DIR/DATA

if [ ! -d "$DATA_DIR" ]; then
  echo "THERE IS NO DATA DIR IN $RES_DIR"
  exit 1
fi

cd $DATA_DIR

# list all files in data file
ls -fp | grep -v / > $DATA_FILE

echo "Writing to $DATA_FILE"

# shuffle
perl -MList::Util -e 'print List::Util::shuffle <>' $DATA_FILE > $DATA_FILE.new
mv $DATA_FILE.new $DATA_FILE

echo "Shuffled $DATA_FILE"

