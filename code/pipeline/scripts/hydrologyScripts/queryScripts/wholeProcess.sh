#!/bin/bash

# This script submits the job line

FIRST=$(qsub runDijkstra.hydrology.sh)
qsub -W depend=afterany:$FIRST clusterHypo.sh
SECOND=$(qsub -W depend=afterany:$FIRST makeBags.hydrology.sh)
qsub -W depend=afterany:$SECOND batchPLDA.hydrology.sh
