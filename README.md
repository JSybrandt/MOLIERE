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
