CC = gcc
CFLAGS = -Wall -g -Werror -pedantic -std=gnu18
PROJ = ush
PACKNAME = $(PROJ).tar.gz 

all:
	make ush

ush: ush.c ush.h
	$(CC) -o $@ $(CFLAGS) $<

run: ush
	./$<

pack: ush.c ush.h Makefile README.md
	tar -czf $(PACKNAME) $^

unpack: $(PACKNAME)
	tar -xzf $<
