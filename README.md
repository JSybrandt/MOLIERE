# Important Update

We've moved development of Moliere over to the new [PyMoliere][https://github.com/JSybrandt/PyMoliere] repo! This new version is faster, uses updated technology, and is easier to use.


![moliere_logo][moliere_logo]

This repo contains the code described in the publication: *MOLIERE: Automatic Biomedical Hypothesis Generation System*

Don't have a super computer? [Request your query here!][moliere_query]

We organize our code into two major sub-projects, `build_network` and `run_query`.

As the names imply, you will need to run the former in order to construct a dataset that the latter can use.
Further details can be found in each sub-projects own ReadMe.md.
But, in short, running either python script with the `-h` flag will print our all command-line arguments for each utility.
This should get you pointed in the right direction.

Note, please add `export MOLIERE_HOME=<location of this git repo>` to your bashrc.
Both sub-projects are going to expect it.

If you have any questions, feel free to reach out to:

jsybran [at] clemson [dot] edu

Or just leave an issue in this repo.


## Installation

First, make sure you clone this project with the `--recursive` flag.
This initializes my submodules present in the `./external` directory.

Then, make sure you have the following dependencies:

```
  gcc >= 5.4
  python 3
  java 1.8     (for AutoPhrase)
  scons        (for NetworKit)
  google test  (for NetworKit)
  mpich 3.1.4  (for PLDA)
```

Ideally, a simple `make` while in the project directory should do the trick.
Often, errors are due to our NetworKit dependency, so please refer to their readme if you have any issues.

## Runtime Requirements

If you are constructing the Moliere Knowledge Network, please note that our code is extremely memory intensive if you choose to follow the default construction parameters.
After downloading MEDLINE and performing an initial parsing, the `build_network.py` script then sends all text through AutoPhrase.
This often requires over 1 terabyte of memory to complete.
Then, we send the text through fasttext in order to define embeddings for each word.
This too is memory intensive, on the order of 100's of Gb.

*Note:* If you do not have the resources to construct the Moliere Knowledge Network, we keep our latest public release [Available Online][moliere_data].

When running a query, we expect the knowledge network data to be stored on a filesystem that allows for parallel reads.
If this is not available, you may need to set the environment variable `OMP_NUM_THREADS=1` in order to disable our multi-threaded reads.
We rely on a parallel file system in order to keep query-time memory usage down by "skimming" our large network files in an efficient manner.
This way, we only keep relevant information in memory, which drastically improves runtime.

*Note:* If you do not have the resources to run a query, we allow for query requests [Available Online][moliere_query].

## Citation

If you use or reference any of this work, please cite the following paper:

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

[moliere_logo]:http://sybrandt.com/img/logo/moliere_logo.png
[moliere_query]:http://sybrandt.com/post/moliere-run-query/
[moliere_data]:https://drive.google.com/open?id=1mHTcKvt6EhkHhDjQlHZ7S0T3WGZZ6K5E
