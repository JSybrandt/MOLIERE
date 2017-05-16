#PBS -N standardize6
#PBS -l select=1:ncpus=8:mem=10gb,walltime=72:00:00
#PBS -o /home/jsybran/job.out
#PBS -e /home/jsybran/job.err
#PBS -M jsybran@clemson.edu
#PBS -m ea
#PBS -J 601-700

# note: medline xml files are indexed starting with medline17n0001.xml and ending with medline17n0892.xml

FILE_NUM=$(printf "%04d" ${PBS_ARRAY_INDEX})
IN_FILE_NAME=/scratch2/jsybran/medline/medline17n$FILE_NUM.xml
OUT_FILE_NAME=/scratch2/jsybran/medlineStandardized/file$FILE_NUM.txt

/home/jsybran/projects/preprocessXML/xml2DataFile.py $IN_FILE_NAME $OUT_FILE_NAME
