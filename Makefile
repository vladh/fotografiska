# © 2022 Vlad-Stefan Harbuz <vlad@vladh.net>
# SPDX-License-Identifier: blessing

.PHONY: unix windows

unix:
	gcc fotografiska.c -o bin/fotografiska -lexif -g -Wall -Wno-format-overflow -Wno-unused-variable -std=c99

windows:
	gcc fotografiska.c -o bin/fotografiska -lexif-12 -g -Wall -Wno-format-overflow -Wno-unused-variable -std=c99
