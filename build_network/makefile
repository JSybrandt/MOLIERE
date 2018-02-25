CC=g++

NETWORKIT=./external/networkit

LINK_PATHS=
INCLUDE_PATHS=-I./../shared_headers
INCLUDE_PATHS+=-I./external/pugixml/src
LIB=-fopenmp
CPPV=-std=c++11
WARN=-Wall
OPT=-O3


B=./bin
S=./src
E=./external


NO_COLOR=\x1b[0m
WARN_COLOR=\x1b[33;01m


all: $B/parseXML $B/fasttext

$B/parseXML: $S/parseXML/main.cpp $B/pugixml.o
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/pugixml.o: $E/pugixml/src/pugixml.cpp
	$(CC) -c -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/fasttext:
	make -C $E/fastText -j
	cp $E/fastText/fasttext $B/fasttext


clean:
	rm $B/*
	make -C $E/fastText clean
