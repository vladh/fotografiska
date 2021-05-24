.PHONY: all run

all:
	gcc fotografiska.c -o bin/fotografiska

run: all
	./bin/fotografiska
