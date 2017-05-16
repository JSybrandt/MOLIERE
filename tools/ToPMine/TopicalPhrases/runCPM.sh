#!/bin/bash

binFolder="bin"
classpath="$binFolder"
className="phraseMining/MineContiguousPatterns"

inFolder="input_dataset/"  #input folder, don't forger "/"
outFolder="input_dataset_output/" #output folder don't forget "/"
dataName="input" # dataset name
minsup=$1
testNum=0	#number of test documents
thresh=$3
maxPattern=$2
echo $inFolder
echo $outFolder
echo $dataName
echo $minsup
echo $testNum

java -Xmx300g -cp $classpath  $className $inFolder $outFolder $dataName $minsup $testNum $thresh $maxPattern
