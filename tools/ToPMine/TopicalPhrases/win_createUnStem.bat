
set binFolder=bin
set jarFolder=lib\*
set classpath=%binFolder%;%jarFolder%



set parFile=input_dataset_output\input_partitionedTraining.txt
set canFile=input_dataset_output\candidate

set stopWordFile=stoplists\en.txt
set vocFile=input_dataset\input_vocFile
set rawFile=%1%
set outFile=input_dataset_output\unmapped_phrases
set maxPattern=%2%




set className=unStem.CandidatePhraseGen
java -Xmx4g -cp %classpath%  %className% %parFile% %canFile%


set className=unStem.UnStemPhraseClass
java -Xmx6g -cp %classpath%  %className% %canFile% %stopWordFile% %vocFile% %rawFile% %outFile% %maxPattern%