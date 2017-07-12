#!/bin/bash

# This script submits the job line

FIRST=$(qsub runDijkstra.hydrology.sh)
qsub -W depend=afterok:$FIRST clusterHypo.sh
SECOND=$(qsub -W depend=afterok:$FIRST makeBags.hydrology.sh)
qsub -W depend=afterok:$SECOND batchPLDA.hydrology.sh
