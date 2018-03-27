B=./bin
S=./src
E=../external
H=../shared_headers

CC=g++

INCLUDE_PATHS+=-I$H
LIB=-fopenmp
CPPV=-std=c++11
WARN=-Wall
OPT=-O3

all: $B/getWordCount $B/ldaView2Model

$B/getWordCount: $S/getWordCount/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$B/ldaView2Model: $S/ldaView2Model/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)
