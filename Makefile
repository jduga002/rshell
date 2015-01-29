# Makefile for rshell

FLAGS = -Wall -Werror -ansi -pedantic

all: rshell ls | bin

rshell: | bin
	g++ $(FLAGS) -o bin/rshell src/main.cpp

ls: | bin
	g++ $(FLAGS) -o bin/ls src/ls.cpp

bin:
	mkdir bin
