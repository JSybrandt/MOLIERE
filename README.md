![moliere_logo](https://github.com/JSybrandt/jsybrandt.github.io/blob/master/img/logo/moliere_logo.png?raw=true)

This repo contains the code described in the publication: *MOLIERE: Automatic Biomedical Hypothesis Generation System*

Warning, this code does not contain the datasets used to train the network.
These files can be obtained from PubMed.

The tools directory contains a slightly modified version of ToPMine, a utility which finds meaningful n-grams. The modification simply stops ToPMine from removing tokens which contain numbers, very important for gene names.

Then in the code directory, there are sub directories wherein each component used to create and query the network can be found.

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


