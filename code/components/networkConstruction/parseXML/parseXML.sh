#!/bin/bash
PROJ_HOME=$(pwd | grep -o .*moliere)
PARSED_DATA=$PROJ_HOME/data/parsedData
RAW_DATA=$PROJ_HOME/data/rawData

mkdir -p $PARSED_DATA


module load gnu-parallel

parallel "./parseXML -v -f $RAW_DATA/{/} -o $PARSED_DATA/{/}" ::: $RAW_DATA/*
