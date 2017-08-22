#!/bin/bash
module load gnu-parallel
module load gcc
module load mpich2

QUERY_RES_FILE="/scratch2/jsybran/crisprRes.data"
MODEL_DIR="/scratch2/jsybran/plda/model"
DATA_DIR="/scratch2/jsybran/plda/data"
VIEW_DIR="/scratch2/jsybran/plda/view"
PMID_BAG_FILE="/scratch2/jsybran/medlineNetworkData/pmidBags"

parallel -j8 runPLDA1 {} $MODEL_DIR/{/}  ::: $DATA_DIR/*

parallel view_model.py {} $VIEW_DIR/{/} ::: $MODEL_DIR/*


