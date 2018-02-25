PROJ_HOME=$(shell pwd | grep -o .*moliere)

FLANN_LIB=~/lib
FLANN_INCLUDE=~/lib/include

CC=g++

FLAGS=-std=c++11 -fopenmp -O3
# FLAGS+=-Wall -Wextra -Werror -pedantic -Wundef -Wshadow
# FLAGS+=-Wcast-align -Wcast-qual -Wold-style-cast
FLAGS+=-I$(PROJ_HOME)/code/components/sharedHeaders
FLAGS+=-lflann -L$(FLANN_LIB) -I$(FLANN_INCLUDE)


testNNN: testNNN.cpp
	$(CC) $(FLAGS) -o $@ $<

clean:
	rm -f testNNN

