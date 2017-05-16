#!/bin/bash

binFolder="bin"
jarFolder="lib/mallet.jar:lib/snowball-20051019.jar:lib/trove-2.0.2.jar"
classpath="$binFolder:$jarFolder"


#java arguments
inputFile=$1 #the raw text file
datasetName="input"; # create a folder with the same name, and store all files under this folder
minsup=3	# the minimal support a word needs to satisfy
startsWithID=1 # 1 -> no doc id/no doc label;  2 -> doc id/no doc label; 3 -> with doc id and doc label
stopwordFile='stoplists/en.txt'
	
echo $inputFile
echo $datasetName
echo $minsup
echo $startsWithID


#first run prepare data
echo ___
className="DataPreparation/PrepareData"
java -Xmx300g -cp $classpath  $className $inputFile $datasetName $minsup $startsWithID $stopwordFile


#second run parepare partition
echo ___
className="DataPreparation/PreparePartitionFile"
java -Xmx300g -cp $classpath  $className $inputFile $datasetName $startsWithID
echo ___

