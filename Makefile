.PHONY: unix run

unix:
	gcc fotografiska.c -o bin/fotografiska -lexif -g -Wall -Wno-format-overflow -Wno-unused-variable

windows:
	gcc fotografiska.c -o bin/fotografiska -lexif-12 -g -Wall -Wno-format-overflow -Wno-unused-variable
