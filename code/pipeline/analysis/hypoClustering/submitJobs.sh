#!/bin/bash

# MUST SET PARAM
#K=2
#DATA=/scratch2/jsybran/ddx3/rootCluster

# GOTO dir with scripts
# pushd /zfs/safrolab/users/jsybran/moliere/code/pipeline/scripts/hypoClustering


FIRST=$(qsub -v K=$K,DATA=$DATA generateFactors.sh)
#SECOND=$(qsub -v K=$K,DATA=$DATA -W depend=afterany:$FIRST analyzeFactors.sh)
#THIRD=$(qsub -v K=$K,DATA=$DATA -W depend=afterany:$FIRST splitByFactor.sh)
