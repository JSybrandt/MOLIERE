#!/bin/bash
PROJ_HOME=$(pwd | grep -o .*moliere)
PARSED_DATA=$PROJ_HOME/data/parsedData
RAW_DATA=$PROJ_HOME/data/rawData
PROC_TEXT=$PROJ_HOME/data/processedText
CANON=$PROC_TEXT/canon.txt

mkdir -p $PARSED_DATA


module load gnu-parallel

parallel "./parseXML -v -f $RAW_DATA/{/} -o $PARSED_DATA/{/.}.txt" ::: $RAW_DATA/*

echo "cat together"
rm $CANON
cat $PARSED_DATA/* >> $CANON
