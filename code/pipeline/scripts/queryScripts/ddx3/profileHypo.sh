#/bin/bash
#PBS -N ddx3profile
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/ddx3profile.out
#PBS -e /home/jsybran/jobLogs/ddx3profile.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

module load gcc python

DATA=/scratch2/jsybran/ddx3
DIJK_FILE=$DATA/ddx3.dijkstra
LABEL_FILE=$DATA/final.labels
EDGE_FILE=$DATA/final.edges
CENT_FILE=$DATA/centrality.data
VIEW_PATH=$DATA/VIEW_FILES
DATA_PATH=$DATA/DATA_FILES
OUT_PATH=$DATA/profile.data

#  usage: profileHypotheses.py [-h] [-e EDGEPATH] [-l LABELPATH] [-d DIJKPATH]
#                              [-c CENTRALITYPATH] [-V VIEWDIRPATH]
#                              [-D DATADIRPATH] [-o OUTPATH] [-C COHERENCECUTOFF]
#                              [-v]
#
#  optional arguments:
#    -h, --help            show this help message and exit
#    -e EDGEPATH, --edge-file EDGEPATH
#                          file path of the FULL edge list.
#    -l LABELPATH, --label-file LABELPATH
#                          file path of the FULL label file.
#    -d DIJKPATH, --dijkstra-file DIJKPATH
#                          file path of the dijkstra file.
#    -c CENTRALITYPATH, --centrality-file CENTRALITYPATH
#                          file path of the dijkstra file.
#    -V VIEWDIRPATH, --view-dir VIEWDIRPATH
#                          directory path of topic model views.
#    -D DATADIRPATH, --data-dir DATADIRPATH
#                          directory path of bag of words files.
#    -o OUTPATH, --out-file OUTPATH
#                          file path of the resulting profile file.
#    -C COHERENCECUTOFF, --coherence-cutoff COHERENCECUTOFF
#                          Number of words to consider when calculating
#                          coherence.
#    -v, --verbose         print debug info

profileHypotheses.py -e $EDGE_FILE -l $LABEL_FILE -d $DIJK_FILE -c $CENT_FILE -V $VIEW_PATH -D $DATA_PATH -o $OUT_PATH
