@echo off

set inputFile=%1%

set binFolder=bin
set jarFolder=lib\*
set classpath=%binFolder%;%jarFolder%

set datasetName=input
set minsup=3
set startsWithID=1
set stopwordFile=stoplists\en.txt
set className=DataPreparation.PrepareData

java -Xmx6g -cp %classpath%  %className% %inputFile% %datasetName% %minsup% %startsWithID% %stopwordFile%

set className=DataPreparation.PreparePartitionFile
java -Xmx6g -cp %classpath%  %className% %inputFile% %datasetName% %startsWithID%