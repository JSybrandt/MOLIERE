#!/bin/bash

binFolder="bin"
jarFolder="lib/mallet.jar:lib/snowball-20051019.jar:lib/trove-2.0.2.jar"
classpath="$binFolder:$jarFolder"


#java arguments
parFile="input_dataset_output/input_partitionedTraining.txt"	#partitio file
canFile="input_dataset_output/candidate"	#candidate file

stopWordFile="stoplists/en.txt" #stop words file
vocFile="input_dataset/input_vocFile"      #voc file
rawFile=$1      #raw file
outFile="input_dataset_output/unmapped_phrases"
maxPattern=$2




className="unStem/CandidatePhraseGen"
java -Xmx300g -cp $classpath  $className $parFile $canFile


className="unStem/UnStemPhraseClass"
java -Xmx300g -cp $classpath $className $canFile $stopWordFile $vocFile $rawFile $outFile $maxPattern
