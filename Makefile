.PHONY: all test run

all:
	gcc src/fotografiska.c -o bin/fotografiska -lexif -g -Wall -Wno-format-overflow -Wno-unused-variable

test:
	gcc pstr/pstr_test.c -o bin/pstr_test -g -Wall -Wno-format-overflow -Wno-unused-variable

run: all
	./bin/fotografiska

run-test: test
	./bin/pstr_test
