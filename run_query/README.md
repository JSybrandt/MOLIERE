# MOLIERE QUERY RUNNER

This repo stores a polished subset of the main MOLIERE code, and provides a simple python interface to query the MOLIERE knowledge network for new hypotheses.

## System Requirements

 - gcc (something recent enough to support c++11)
 - mpich (mpich 1, NOT 2, needs to provide mpicxx)
 - python 3
 - _preferred:_  some sort of parallel file system

To run on Palmetto simply use `module load gcc mpich python`.

## Install Process

The install process is pretty informal.
The following steps are how I would setup Moliere on a linux system.

```bash
export MOLIERE_HOME=<some install directory>
mkdir -p $MOLIERE_HOME
cd $MOLIERE_HOME
git clone https://github.com/JSybrandt/Moliere_Query_Runner .
make
```

Next, you just need to get the data file into `$MOLIERE_HOME/data`.
The latest data we have avalible can be found in the following link:

(Depricated:)
[Get Data Here (Google Drive Link)](https://drive.google.com/drive/u/0/folders/0B2hkrBZ0Qc40VXNwcGQ1eEtMTDg)


## Running

Okay, now you have everything you need, assuming you ran make, setup the `$MOLIERE_HOME` environment variable, and downloaded the provided data in to `$MOLIERE_HOME/data`.
To run our system, you will be executing `runQuery.py`.
Note, feel free to move this file anywhere you would like, as long as `$MOLIERE_HOME` is set, you're good to go.

If you run `runQuery.py -h` you can see the options for the system.
But, the general usage is going to be `runQuery.py -n $TOPIC_COUNT -m $TERM_A $TERM_B`.
Note that -n sets the number of topics (defaults to 100) and -m moves the resulting topic model and evaluation files from the local cache (default to /tmp) to the working directory.

## Citation

The work shown here is primarily based off [this paper](https://dl.acm.org/citation.cfm?id=3098057) so if you use our tool in research, please include the following citation:

```
@inproceedings{Sybrandt:2017:MAB:3097983.3098057,
 author = {Sybrandt, Justin and Shtutman, Michael and Safro, Ilya},
 title = {MOLIERE: Automatic Biomedical Hypothesis Generation System},
 booktitle = {Proceedings of the 23rd ACM SIGKDD International Conference on Knowledge Discovery and Data Mining},
 series = {KDD '17},
 year = {2017},
 isbn = {978-1-4503-4887-4},
 location = {Halifax, NS, Canada},
 pages = {1633--1642},
 numpages = {10},
 url = {http://doi.acm.org/10.1145/3097983.3098057},
 doi = {10.1145/3097983.3098057},
 acmid = {3098057},
 publisher = {ACM},
 address = {New York, NY, USA},
 keywords = {hypothesis generation, mining scientific publications, topic modeling, undiscovered public knowledge},
}
```





