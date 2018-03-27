B=./bin
S=./src
E=../external
H=../shared_headers

CC=g++

NETWORKIT=$E/networkit

INCLUDE_PATHS=-I$(NETWORKIT)/include
INCLUDE_PATHS+=-I$H
LINK_PATHS=-B$(NETWORKIT)
LIB=-fopenmp -lNetworKit
CPPV=-std=c++11
WARN=-Wall
OPT=-O3

all: $B/getWordCount $B/ldaView2Model $B/getPapers

$B/getWordCount: $S/getWordCount/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$B/ldaView2Model: $S/ldaView2Model/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$B/getPapers: $S/getPapers/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)
