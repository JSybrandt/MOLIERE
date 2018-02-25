QSUB=/opt/pbs/default/bin/qsub
RUN=./runExp.array.sh

#$QSUB -v RF=real,EXP=fulltext $RUN
$QSUB -v RF=fake,EXP=fulltext $RUN
#$QSUB -v RF=real,EXP=abstract $RUN
#$QSUB -v RF=fake,EXP=abstract $RUN
#$QSUB -v RF=real,EXP=hybrid $RUN
#$QSUB -v RF=fake,EXP=hybrid $RUN
