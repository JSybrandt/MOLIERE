#!/bin/bash

module load java

export CLASSPATH=/zfs/safrolab/users/jsybran/moliere/tools/ToPMine/TopicalPhrases/src/DataPreparation/libs/libstemmer_java/java/libstemmer.jar:/zfs/safrolab/users/jsybran/moliere/tools/ToPMine/TopicalPhrases/src/DataPreparation/libs/mallet-2.0.8/dist/mallet.jar:/zfs/safrolab/users/jsybran/moliere/tools/ToPMine/TopicalPhrases/src/DataPreparation/libs/mallet-2.0.8/dist/mallet-deps.jar

javac ./PrepareData.java
