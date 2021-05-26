.PHONY: all run

all:
	gcc fotografiska.c -o bin/fotografiska -lexif -g -Wall

run: all
	./bin/fotografiska
