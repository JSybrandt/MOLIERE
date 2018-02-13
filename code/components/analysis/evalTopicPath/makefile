PROJ_HOME=$(shell pwd | grep -o .*moliere)


NETWORKIT=$(PROJ_HOME)/code/components/external/source/networkit/

CC=g++

INCLUDE_PATHS=-I$(NETWORKIT)/include
INCLUDE_PATHS+=-I$(PROJ_HOME)/code/components/sharedHeaders
LINK_PATHS=-L$(NETWORKIT)
LIB=-lNetworKit -fopenmp
CPPV=-std=c++11
WARN=-Wall
OPT=-O3


NAME="evalTopicPath"

all: bin bin/$(NAME)

bin/$(NAME): main.cpp
	$(CC) -o $@ $(CPPV) $(WARN) $(OPT) $(INCLUDE_PATHS) $(LINK_PATHS) $< $(LIB)
	ln -sf $@

test: testNetworkit.cpp
	$(CC) -o $@ -std=c++11 -I$(NETWORKIT)/include -L$(NETWORKIT) $< -lNetworKit -fopenmp

bin:
	mkdir -p bin

clean:
	rm -rf bin
	rm -f *.o
	rm -f $(NAME)

