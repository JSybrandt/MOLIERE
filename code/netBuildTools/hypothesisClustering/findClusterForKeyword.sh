#!/bin/bash

CLUSTER_DIR=$1
KEYWORD1=${2-"."}
KEYWORD2=${3-"."}

cd $CLUSTER_DIR


echo Searching for $KEYWORD1 and $KEYWORD2
echo -e "ClutserNum. \t FracInCluster \t FracTotal"
echo    "=========================================="
useageCount=$(find . | grep $KEYWORD1 | grep $KEYWORD2 | wc -l)
for cluster in $(ls); do
  clusterTotal=$(ls -f $cluster | wc -l)
  clusterTotal=$(($clusterTotal - 2))
  if [ $clusterTotal -gt 0 ]; then
    part=$(ls -f $cluster | grep $KEYWORD1 | grep $KEYWORD2 | wc -l)
    echo -e "$cluster \t $(bc <<< "scale=4; $part/$clusterTotal") \t\t $(bc <<< "scale=4; $part/$useageCount")"
  fi
done | sort -k 3,3 -r -n
