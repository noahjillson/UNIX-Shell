CC = gcc
CFLAGS = -Wall -Werror -pedantic -std=gnu18
LOGIN = jillson
SUBMITPATH = ~cs537-1/handin/$(LOGIN)/P3
SUBMITFILE = $(LOGIN).tar.gz 

all:
	make wsh

wsh: wsh.c wsh.h
	$(CC) -o $@ $(CFLAGS) $<

run: wsh
	./$<

pack: wsh.c wsh.h Makefile README.md
	tar -czf $(SUBMITFILE) $^

submit: pack
	cp $(SUBMITFILE) $(SUBMITPATH)
