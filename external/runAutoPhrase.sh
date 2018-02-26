#!/bin/bash


export MODEL=MOLIERE
export RAW_TRAIN=$1
export FIRST_RUN=1
export ENABLE_POS_TAGGING=0
export THREAD=$2
export RAW_LABEL_FILE=$3

if [ -z "$RAW_TRAIN" ]; then
  echo "Must supply abstract file as arg 1"
  exit 1
fi
if [ -z "$THREAD" ]; then
  echo "Must supply thread count as arg 2"
  exit 1
fi
if [ -z "$RAW_LABEL_FILE" ]; then
  echo "Must supply expert labels as $3"
  exit 1
fi

cd AutoPhrase
./auto_phrase.sh
