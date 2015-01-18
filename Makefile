# Makefile for rshell

CXXFLAGS = -Wall -Werror -ansi -pedantic

all: | bin
	g++ -o bin/rshell src/main.cpp

rshell:
	g++ -o bin/rshell src/main.cpp

bin:
	mkdir bin
