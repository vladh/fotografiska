.PHONY: all run

all:
	gcc fotografiska.c -o bin/fotografiska -O2 -g -Wall -Werror

run: all
	./bin/fotografiska
