
**** Illimine Copyright ****

University of Illinois at Urbana-Champaign, 2014

illimine.cs.illinois.edu


**** Additional Copyright ****

This package contains the source code and the dataset used in the following paper:

@inproceedings{conf/vldb/ElkishkySWVH13,
  author    = {Ahmed El-Kishky and
		Yanglei Song and
		Chi Wang and
		Clare R. Voss and
		Jiawei Han},
  title     = {Scalable Topical Phrase Mining from Text Corpora},
  booktitle = {VLDB},
  year      = {2015},
}

If you use any contents in this package, please cite the above paper as your reference.


**** Code explanation ****

Place the input corpus in rawFiles with each document on a line
Edit the parameters in run.sh(linux)/win_run.bat(for windows) for minimum support and topic modeling parameters
run run.sh (for linux)/ win_run.bat (for windows, or just double click)

Parameter explanation:
1.) minsup = minumum support (minimum times a phrase candidate should appear in the corpus to be significant)
2.) maxpattern = max size you would like a phrase to be (if you don't want too long of phrases that occasionally occur)
3.) numTopics = number of topics - same as LDA
4.) Gibbs Sampling iterations = number of iterations done for inference (learning the parameters. Usually 500 is good, may do more if you like)
5.) thresh (significance) = the significance of a phrase. Equivalent to a z-score. I usually use 3 to 5. The higher it is, the fewer phrases will be found, but they will be of very high quality.
6.) Topic Model, two variants of PhraseLDA are used (choose 1 or 2). 2 is the default topic model.

The output should be in output, you will have corpus and topics

Corpus is the corpus partitioned into phrases, stopwords are removed except those in phrases
each phrase is delimited by a comma

The topics file is a one-to-one and onto mapping between documents and topics for each phrase

e.g

line 1 in corpus is document 1
line 1 in topics is the topics for phrases in document 1

example:

Line 1 corpus: Cow Dog Flower
Line 1 topics: 2 2 1

Cow is topic 2, dog is topic 2, flower is topic 1 

there are also output files for the top phrases (ranked by their frequency)

 	
**** For More Questions ****

Please contact illimine.cs.illinois.edu or Ahmed El-Kishky (elkishk2@illinois.edu) or
Yanglei Song (ysong44@illinois.edu)
