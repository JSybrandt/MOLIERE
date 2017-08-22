#/bin/bash
#PBS -N mergeUMLS
#PBS -l select=1:ncpus=24:mem=500gb,walltime=72:00:00
#PBS -q bigmem
#PBS -o /home/jsybran/jobLogs/mergeUMLS.out
#PBS -e /home/jsybran/jobLogs/mergeUMLS.err
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

DATA=/zfs/safrolab/users/jsybran/moliere/data
NET_A=$DATA/network/centAndPhrases
NET_B=$DATA/network/umls
AB_EDGES=$DATA/network/keyword2terms
RES=$DATA/network/final

# The idea here is to give a light weight to inter-abstract edges, a medium weight to A-T edges, and a large weight to T-T edges.
mergeNets -a $NET_A.edges -A $NET_A.labels -x 1 -b $NET_B.edges -B $NET_B.labels -y 2.5 -c $AB_EDGES.edges -z 0 -r $RES.edges -R $RES.labels
