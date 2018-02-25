#!/bin/bash

module load gcc

./tfidf -d example/doc.txt -t example/terms.txt -o example/edges.data -ivn
