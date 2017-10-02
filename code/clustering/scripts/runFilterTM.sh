#!/bin/bash
view_path=$1

for view in `ls $view_path`; 
do 
  echo $view_path"/"$view" is processing..." ; 
  grep -A 21 TOPIC $view_path"/"$view  > $view_path"/"$view"_filtered"
done
