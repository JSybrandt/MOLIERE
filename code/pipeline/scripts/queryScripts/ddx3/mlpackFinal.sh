#!/bin/bash

pushd /zfs/safrolab/users/jsybran/moliere/code/netBuildTools/hypothesisClustering/

./mlpack_finalize.jl -d /scratch2/jsybran/ddx3/nmfRes/k-5/ -W /scratch2/jsybran/ddx3/nmfRes/k-5-res/W.pgm -H /scratch2/jsybran/ddx3/nmfRes/k-5-res/H.pgm -X /scratch2/jsybran/ddx3/ddx3SimMat.pgm -C /scratch2/jsybran/ddx3/nmfRes/k-5-res/cluster.data -l /scratch2/jsybran/ddx3/matrixLabels.txt -k 5 -w -v

