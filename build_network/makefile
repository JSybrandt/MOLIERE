B=./bin
S=./src
E=../external
H=../shared_headers

CC=g++

LINK_PATHS=-L $B
LINK_PATHS+=-L $E/flann/build/lib
INCLUDE_PATHS=-I$H
INCLUDE_PATHS+=-I$E/pugixml/src
INCLUDE_PATHS+=-I$E/porter2_stemmer
INCLUDE_PATHS+=-I$E/flann/src/cpp
LIB=-fopenmp -l flann
CPPV=-std=c++11
WARN=-Wall
OPT=-O3

NO_COLOR=\x1b[0m
WARN_COLOR=\x1b[33;01m


all: $B/parseXML $B/fasttext $B/cleanRawText $B/apHighlight2Underscore $B/parseUmlsText $B/parseUmlsEdges $B/makeApproxNNN $B/makeDocumentEdges $B/finalizeNetwork $B/filterWordsByCount $B/centroidFromNeighbors $B/removeShortDocuments

$B/parseXML: $S/parseXML/main.cpp $B/pugixml.o $B/porter2_stemmer.o
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/pugixml.o: $E/pugixml/src/pugixml.cpp
	$(CC) -c -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/fasttext: $E/fastText/fasttext
	ln -s -f $$(readlink -f $E/fastText/fasttext) $B/fasttext

$E/fastText/fasttext:
	make -C $E/fastText

$B/cleanRawText: $S/cleanRawText/main.cpp $B/porter2_stemmer.o
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/porter2_stemmer.o: $E/porter2_stemmer/porter2_stemmer.cpp
	$(CC) -c -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/apHighlight2Underscore: $S/apHighlight2Underscore/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/filterWordsByCount: $S/filterWordsByCount/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/removeShortDocuments: $S/removeShortDocuments/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/centroidFromNeighbors: $S/centroidFromNeighbors/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/makeDocumentEdges: $S/makeDocumentEdges/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/parseUmlsText: $S/parseUmlsText/main.cpp $B/porter2_stemmer.o
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/parseUmlsEdges: $S/parseUmlsEdges/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/finalizeNetwork: $S/finalizeNetwork/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/makeApproxNNN: $S/makeApproxNNN/main.cpp $B/libflann.so
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $^ $(LIB)

$B/libflann.so: $E/flann/build/lib/libflann.so
	ln -s `readlink -f $<` $@

$E/flann/build/lib/libflann.so:
	Please build flann in the external dir


clean:
	rm $B/*
	make -C $E/fastText clean
