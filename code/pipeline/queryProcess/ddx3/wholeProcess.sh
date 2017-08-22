#!/bin/bash

# This script submits the job line

FIRST=$(qsub runDijkstra.sh)
qsub -W depend=afterany:$FIRST clusterHypo.sh
SECOND=$(qsub -W depend=afterany:$FIRST makeBags.sh)
qsub -W depend=afterany:$SECOND batchPLDA.sh
