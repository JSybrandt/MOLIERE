#!/bin/bash
user=`whoami`
hypo_name="cancer"
source_path="/home/"$user"/alg/MOLIERE/code/clustering/scripts/" 
data_path="/scratch2/"$user"/moliere/"$hypo_name"/out/" 
output_path="/home/"$user"/alg/MOLIERE/results/"$hypo_name"/"
out_model="/scratch2/"$user"/plda/model/"$hypo_name"/"
for ds in `ls $data_path`;
do
     exp_info="moliere_plda"$hypo_name"_"$ds"_"`/bin/date +%m%d%y_%H%M`;
     echo $exp_info  >> "./running_processes";
     echo $ds ;
#     echo "data path:"$data_path
     qsub -v user=$user,src_path=$source_path,dataset_name=$ds,model_out=$out_model"_"$ds,output_path=$output_path,experiment_info=$ds,data_path=$data_path worker_plda.pbs
#     echo "qsub -v user=$user,src_path=$source_path,dataset_name=$ds,model_out=$out_model"_"$ds,output_path=$output_path,experiment_info=$ds,data_path=$data_path worker_plda.pbs"
#     exit #this is only for test to run the whole thing once only
done
