Build Moliere Knowledge Network
===============================

The code supplied here performs all the steps of constructing the Moliere Knowledge Network (MKK).

This code requires:

 - Python 3
 - GCC 7.1
 - (recommended) a parallel file system
 - Java 1.8
 - OpenMP
 - gunzip
 - GNU Linux Commands
   - awk
   - sed
   - mv
   - rm
   - cat

Setup
-----

First, make sure all the submodules in `../external` are properly cloned.
Then, run `cd ../external/flann/; mkdir build; cd build; cmake ..; make` to construct flann.
(There is no need to run make install, this code will just pull from the external dir).

Then, you should be able to just run `make` to set everything up.
Lastly, set `MOLIERE_HOME` to the above directory, and `MOLIERE_DATA` to wherever you would like to construct the network.

Running
-------

The basic usage should be as simple as `./build_network.py`.
Doing so starts by downloading all of pubmed from ftp.ncbi.nlm.nih.gov, so warning it will use quite a lot of storage space.

After the download is complete, and the resulting xml files are extracted, this code then extracts each abstract's text and year, cleans and stems, passes the data through autophrase, trains a vector space with fast text, and eventually assembes the whole knowledge network.

UMLS
----

We note that UMLS provides substantial additional data for our network.
But, we cannot automatically download it because of NLM licensing.
So, we advise that you go [HERE](https://www.nlm.nih.gov/research/umls/) to make your free account and download a UMLS metathesaurus release.

After installing the release, please provide `-u` to `./build_network.py` in order to incorporate this data.
Note, the network should construct fine with or without the UMLS data, but the added information will improve results.
