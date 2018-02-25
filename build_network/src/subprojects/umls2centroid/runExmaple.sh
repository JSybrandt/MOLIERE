#!/bin/bash

module load gcc

./connectUMLS -d example/miniUMLS/ -t example/keywordList.labels -o example/out.edges -v
