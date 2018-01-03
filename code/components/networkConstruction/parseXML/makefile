PROJ_HOME=$(shell pwd | grep -o .*moliere)

CC=g++

FLAGS=-std=c++11 -fopenmp -O3
FLAGS+=-Wall -Wextra -Werror -pedantic -Wundef -Wshadow
FLAGS+=-Wcast-align -Wcast-qual -Wold-style-cast
FLAGS+=-I$(PROJ_HOME)/code/components/sharedHeaders


NAME="parseXML"

all: bin bin/$(NAME)

bin/$(NAME): main.cpp pugixml.o
	$(CC) $(FLAGS) -o $@ $^
	ln -sf $@

pugixml.o: pugixml.cpp pugiconfig.hpp
	$(CC) $(FLAGS) -c -o $@ $<

bin:
	mkdir -p bin

clean:
	rm -rf bin
	rm -f *.o
	rm -f $(NAME)
