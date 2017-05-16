set binFolder=bin
set jarFolder=lib\*
set classpath=%binFolder%;%jarFolder%


set trainFile=input_dataset_output\input_partitionedTraining.txt
set testFile=input_dataset_output\input_partitionedTest.txt
set wordTopicAssign=input_dataset_output\input_wordTopicAssign.txt
set topicFile=input_dataset_output\intput_topics.txt
set paraFile=input_dataset_output\input_phrLDA_info.txt
set usePhraseLDA=%1%
set K=%2%
set iter=%3%
set optBurn=%4%
set alpha=%5%
set optInt=%6%
set classname=topicmodel.RunPhraseLDA

java -Xmx6g -cp %classpath%  %classname% %trainFile% %testFile% %wordTopicAssign% %topicFile% %paraFile% %K% %iter% %usePhraseLDA% %optBurn% %alpha% %optInt%

