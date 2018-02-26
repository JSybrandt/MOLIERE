B=./bin
S=./src
E=../external
H=../shared_headers

CC=g++

LINK_PATHS=
INCLUDE_PATHS=-I$H
INCLUDE_PATHS+=-I$E/pugixml/src
INCLUDE_PATHS+=-I$E/porter2_stemmer
LIB=-fopenmp
CPPV=-std=c++11
WARN=-Wall
OPT=-O3

NO_COLOR=\x1b[0m
WARN_COLOR=\x1b[33;01m


all: $B/parseXML $B/fasttext $B/stripStopwords

$B/parseXML: $S/parseXML/main.cpp $B/pugixml.o $B/porter2_stemmer.o
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/pugixml.o: $E/pugixml/src/pugixml.cpp
	$(CC) -c -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/fasttext:
	make -C $E/fastText -j
	cp $E/fastText/fasttext $B/fasttext

$B/stripStopwords: $S/stripStopwords/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$B/porter2_stemmer.o: $E/porter2_stemmer/porter2_stemmer.cpp
	$(CC) -c -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)


clean:
	rm $B/*
	make -C $E/fastText clean
