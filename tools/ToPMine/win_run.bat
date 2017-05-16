@echo off
@set inputFile=C:\Users\Yanglei\Dropbox\PhraseLDAV2\topicalPhrases\rawFiles\mod.txt
@set minsup=30
@set thresh=4
@set maxPattern=3
@set gibbsSamplingIterations=1000
@set topicModel=2

cd TopicalPhrases
echo Data preparing...
call win_runDataPreparation.bat %inputFile%

echo Continuous Pattern Mining ... 
call win_runCPM.bat %minsup% %maxPattern% %thresh%

set numTopics=5
set optimizationBurnIn=100
set alpha=2
set optimizationInterval=50
call win_runPhrLDA.bat %topicModel% %numTopics% %gibbsSamplingIterations% %optimizationBurnIn% %alpha% %optimizationInterval%

call win_createUnStem.bat %inputFile% %maxPattern%
python unMapper.py input_dataset\input_vocFile input_dataset\input_stemMapping input_dataset_output\unmapped_phrases input_dataset_output\input_partitionedTraining.txt input_dataset_output\newPartition.txt

copy input_dataset_output\newPartition.txt ..\output\corpus.txt
copy input_dataset_output\input_wordTopicAssign.txt ..\output\topics.txt
rem rmdir input_dataset /s /q
rem rmdir input_dataset_output /s /q

cd ..
cd output
python topPhrases.py
python topTopics.py
move *.txt outputFiles
cd ..
