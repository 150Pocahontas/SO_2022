### Makefile ###

CC = gcc

CFLAGS = -Wall -g -c

all: server client

server: bin/sdstored

client: bin/sdstore

bin/sdstored: obj/sdstored.o

obj/sdstored.o: src/sdstored.c
	$(CC) src/sdstored.c $(CFLAGS) -o bin/sdstored

bin/sdstore: obj/sdstore.o

obj/sdstore.o: src/sdstore.c
	$(CC) src/sdstore.c $(CFLAGS) -o bin/sdstore

clean:
	rm -r -f obj/* tmp/* bin/*
