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
You should be able to just type 'make' and all utilities contained in `./src/` will be placed into `./bin/`.

Note: this system is going to expect a "data directory."
This is the same directory that `build_network` creates.

If you don't want to build your own network, and instead want to download a couple hundred GB, the following link contains a past version of our data.


(Deprecated: Some files may not have compatible names)
[Get Data Here (Google Drive Link)](https://drive.google.com/drive/u/0/folders/0B2hkrBZ0Qc40VXNwcGQ1eEtMTDg)


## Running

To run our system, you will be executing `runQuery.py`.
Note, feel free to move this file anywhere you would like, as long as `$MOLIERE_HOME` is set, you're good to go.

By default, the code assumes your root data directory is `$MOLIERE_DATA`, and inside that directory we expect the following files:

```
$MOLIERE_DATA/
  processedText/
    abstracts.txt          < This file contains a post-processed version of MEDLINE
  fastText/                < The files here contain vector embeddings for network nodes
    ngram.vec
    pmid.vec
    umls.vec
  network/
    final.bin.edges        < This is a binary edge list file
    final.labels           < This provides names for each node id in the edge list
```

If you run `runQuery.py -h` you can see the options for the system.
But, the general usage is going to be `runQuery.py -n $TOPIC_COUNT -m $TERM_A $TERM_B`.
Note that -n sets the number of topics (defaults to 20) and -m moves the resulting topic model and evaluation files from the local cache (default to /tmp) to the working directory.

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





