# Makefile for rshell

FLAGS = -Wall -Werror -ansi -pedantic

all: rshell ls cp | bin

rshell: | bin
	g++ $(FLAGS) -o bin/rshell src/main.cpp

ls: | bin
	g++ $(FLAGS) -o bin/ls src/ls.cpp

cp: | bin
	g++ $(FLAGS) -o bin/cp src/cp.cpp

bin:
	mkdir bin
