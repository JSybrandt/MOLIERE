#!/bin/bash

module load gcc

PROJ_HOME=$(pwd | grep -o .*/moliere)
PATH=$PATH:$PROJ_HOME/code/components/links

KEY2UML=$PROJ_HOME/data/network/keyword2terms.edges

UMLS_DOCS=$PROJ_HOME/data/tmp/umlsDoc.txt

VEC_DICT=$PROJ_HOME/data/fastText/canon.vec

UMLS_CENTROIDS=$PROJ_HOME/data/fastText/umls.data.NEW

awk '{print $2 " " $1}' $KEY2UML | sort -k1,1 | awk '
BEGIN {umls=""}
$1 == umls { printf " %s", $2 }
$1 != umls { printf "\n%s %s", $1, $2;
             umls = $1;
           }
' > $UMLS_DOCS

makeCentroids -d $VEC_DICT -a $UMLS_DOCS -c $UMLS_CENTROIDS
