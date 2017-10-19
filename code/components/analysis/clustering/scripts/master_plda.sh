#!/bin/bash
user=`whoami`
source_path="/home/"$user"/alg/MOLIERE/code/clustering/scripts/" 
data_path="/scratch2/"$user"/dijk2BoWCorpus/hiv/out/" 
output_path="/home/"$user"/alg/MOLIERE/results/"
out_model="/scratch2/"$user"/plda/model/"
for ds in `ls /scratch2/esadrfa/dijk2BoWCorpus/hiv/out/`;
do
     exp_info="moliere_plda"$ds"_"`/bin/date +%m%d%y_%H%M`;
     echo $exp_info  >> "./running_processes";
#     echo $ds ;
     qsub -v user=$user,src_path=$source_path,dataset_name=$ds,model_out=$out_model"_"$ds,output_path=$output_path,experiment_info=$ds,data_path=$data_path worker_plda.pbs
#     exit #this is only for test to run the whole thing once only
done
