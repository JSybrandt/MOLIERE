#!/bin/bash

binFolder="bin/"
jarFolder="lib/mallet.jar"
classpath="$binFolder:$jarFolder"
classname="topicmodel/RunPhraseLDA"
echo $classpath
titles="input"

trainFile="input_dataset_output/input_partitionedTraining.txt" 		# the training File 	e.g. dblp_subset_partitionedTraining
#trainFile="/Users/ahmed/Dropbox/dataset/dblp_subset_output/dblp_subset_normalLDA.txt"
testFile="input_dataset_output/input_partitionedTest.txt"  		# the test File		e.g. dblp_subset_partitionedTest
wordTopicAssign="input_dataset_output/input_wordTopicAssign.txt"	#the topic assignment of each phrase
topicFile="input_dataset_output/intput_topics.txt"			#the word count(distribution) for each topic
paraFile="input_dataset_output/input_phrLDA_info.txt"			#the information about the phraseLDA
K=$1								# number of topics
iter=$2 								#number of iterations
usePhraseLDA=1 #1 is constraint 2 is phraseLDA
optBurn=$3
alpha=$4
optInt=$5

echo $trainFile
echo $testFile
echo $wordTopicAssign
echo $topicFile
echo $paraFile
echo $K
echo $iter
java -Xmx300g -cp $classpath  $classname $trainFile $testFile $wordTopicAssign $topicFile $paraFile $K $iter $usePhraseLDA $optBurn $alpha $optInt
