CC = gcc
CFLAGS = -Wall -g -Werror -pedantic -std=gnu18
PROJ = wsh
PACKNAME = $(PROJ).tar.gz 

all:
	make wsh

wsh: wsh.c wsh.h
	$(CC) -o $@ $(CFLAGS) $<

run: wsh
	./$<

pack: wsh.c wsh.h Makefile README.md
	tar -czf $(PACKNAME) $^

unpack: $(PACKNAME)
	tar -xzf $<
