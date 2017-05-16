#!/bin/bash

binFolder="bin/"
jarFolder="lib/mallet.jar"
classpath="$binFolder:$jarFolder"
classname="topicmodel/RunPhraseLDA"
echo $classpath

#input
trainFile="subset_abstract_output/subset_abstract_partitionedTraining.txt" 		# the training File 	e.g. dblp_titles_partitionedTraining
testFile="subset_abstract_output/subset_abstract_partitionedTest.txt"  		# the test File		e.g. dblp_titles_partitionedTest

#output
wordTopicAssign="subset_abstract_ConsLDA/subset_abstract_wordTopicAssign.txt"	#the topic assignment of each phrase
topicFile="subset_abstract_ConsLDA/subset_abstract_topics.txt"			#the word count(distribution) for each topic
paraFile="subset_abstract_ConsLDA/subset_abstract_phrLDA_info.txt"			#the information about the phraseLDA
K=50										# number of topics
iter=1000									#number of iterations
usePhraseLDA=1
optBurnin=100
alpha=0.1
optInterval=50
beta=0.005

echo $trainFile
echo $testFile
echo $wordTopicAssign
echo $topicFile
echo $paraFile
echo $K
echo $iter
echo $alpha
echo $beta
java -cp $classpath  $classname $trainFile $testFile $wordTopicAssign $topicFile $paraFile $K $iter $usePhraseLDA $optBurnin $alpha $optInterval $beta
