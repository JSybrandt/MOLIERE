#/bin/bash
#PBS -N mergeNets
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/mergeNets.out
#PBS -e /home/jsybran/jobLogs/mergeNets.err
#PBS -M jsybran@clemson.edu
#PBS -m ea

# usage: ./mergeNets --netA=string --labelA=string --netB=string --labelB=string --netAB=string --resultEdge=string --resultLabel=string [options] ... 
# options:
#   -a, --netA           input network A (int edge file) (string)
#   -A, --labelA         input labels for network A (string)
#   -b, --netB           input network B (int edge file) (string)
#   -B, --labelB         input labels for network B (string)
#   -c, --netAB          crossing edge file (string edge file) (string)
#   -r, --resultEdge     output edge file (string)
#   -R, --resultLabel    output label file (string)
#   -x, --netWeightA     the weight for A edges (float [=1])
#   -y, --netWeightB     the weight for B edges (float [=1])
#   -z, --netWeightAB    the weight for AB edges (float [=1])
#   -v, --verbose        outputs debug information
#   -?, --help           print this message

module load gcc

PATH=$PATH:/zfs/safrolab/users/jsybran/moliere/code/pipeline/tools

DATA=/zfs/safrolab/users/jsybran/moliere/data/hydrologySubset
CENT_FILE=$DATA/network/centroids
KEY_FILE=$DATA/network/terms
TFIDF=$DATA/network/tfidf
RES=$DATA/network/centAndPhrases

# The idea here is to give a light weight to inter-abstract edges, a medium weight to A-T edges, and a large weight to T-T edges.
mergeNets -a $CENT_FILE.edges -A $CENT_FILE.labels -x 1 -b $KEY_FILE.edges -B $KEY_FILE.labels -y 3 -c $TFIDF.edges -z 2 -r $RES.edges -R $RES.labels
