# Makefile for rshell

FLAGS = -Wall -Werror -ansi -pedantic

all: | bin
	g++ $(FLAGS) -o bin/rshell src/main.cpp

rshell: | bin
	g++ -o bin/rshell src/main.cpp

bin:
	mkdir bin
