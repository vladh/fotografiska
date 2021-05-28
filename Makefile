.PHONY: all run

all:
	gcc fotografiska.c -o bin/fotografiska -lexif -g -Wall -Wno-format-overflow -Wno-unused-variable

run: all
	./bin/fotografiska
