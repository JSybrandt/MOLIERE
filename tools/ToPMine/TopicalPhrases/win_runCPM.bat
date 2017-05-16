
set binFolder=bin
set classpath=%binFolder%

set inFolder=input_dataset/
set outFolder=input_dataset_output/
set dataName=input
set minsup=%1%
set maxPattern=%2%
set thresh=%3%
set testNum=0


set className=phraseMining.MineContiguousPatterns
java -Xmx6g -cp %classpath%  %className% %inFolder% %outFolder% %dataName% %minsup% %testNum% %thresh% %maxPattern%