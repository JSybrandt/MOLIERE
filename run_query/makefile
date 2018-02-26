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


all: $B/evalHybrid.py $B/cloud2Bag $B/findCloud $B/findPath $B/mpi_lda $B/view_model.py tools

$B/cloud2Bag: $S/cloud2Bag/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$B/findCloud: $S/findCloud/main.cpp $S/findCloud/graph.h
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$B/findPath: $S/findPath/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$B/mpi_lda:
	make -C $E/plda
	cp $E/plda/mpi_lda $@

$B/view_model.py: $E/plda/view_model.py
	cp $< $@

$B/evalHybrid.py: $S/eval/evalHybrid.py $B/eval_l2 $B/eval_topic_path $B/eval_tpw $B/eval_twe
	cp $< $@

$B/eval_l2: $S/eval/l2/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$B/eval_topic_path: $S/eval/topic_path/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$B/eval_tpw: $S/eval/tpw/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$B/eval_twe: $S/eval/twe/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

tools: $B/getNearestNeighbor $B/getBestPaperFromTM

$B/getBestPaperFromTM: $S/tools/getBestPaperFromTM/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

$B/getNearestNeighbor: $S/tools/getNearestNeighbor/main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)

clean:
	rm $B/*
	make -C $E/plda clean
