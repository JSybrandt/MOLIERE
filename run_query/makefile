B=./bin
S=./src
E=../external
H=../shared_headers

CC=g++

NETWORKIT=$E/networkit

INCLUDE_PATHS=-I$(NETWORKIT)/include
INCLUDE_PATHS+=-I$H
LINK_PATHS=-B$(NETWORKIT)
LIB=-lNetworKit -fopenmp
CPPV=-std=c++11
WARN=-Wall
OPT=-O3




NO_COLOR=\x1b[0m
WARN_COLOR=\x1b[33;01m


all: $B/cloud2Bag $B/findCloud $B/findPath $B/mpi_lda $B/view_model.py $B/evaluate $B/getPapers $B/infer

$B/cloud2Bag: $S/cloud2Bag/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$B/findCloud: $S/findCloud/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$B/findPath: $S/findPath/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$B/getPapers: $S/getPapers/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$B/mpi_lda:
	make -C $E/plda
	cp $E/plda/mpi_lda $@

#relies on mpi_lda making the plda project
$B/infer: $B/mpi_lda
	cp $E/plda/infer $@

$B/view_model.py: $E/plda/view_model.py
	cp $< $@

$B/evaluate: $S/evaluate/main.cpp $(NETWORKIT)/libNetworKit.a
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$(NETWORKIT)/libNetworKit.a:
	cd $(NETWORKIT)
	scons --optimize=Opt --target=Lib --std=c++1
	# We intentionally do not clean this lib, it takes forever to build

clean:
	rm $B/*
	make -C $E/plda clean
