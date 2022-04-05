### Makefile ###

CC = gcc
CFLAGS = -Wall -g -fno-stack-protector
INCLUDE = -I src/include

all : fifo trasnformations server client

fifo: src/mkfifo.c
	gcc -g src/mkfifo.c -o bin/fifo
	./bin/fifo

server: bin/sdstored

client: bin/sdstore

bin/sdstored: obj/sdstored.o
	$(CC) $(CFLAGS) $(INCLUDE) src/readln.c obj/sdstored.o -o bin/sdstored

obj/sdstored.o: src/sdstored.c
	$(CC) $(CFLAGS) $(INCLUDE) -c src/sdstored.c -o obj/sdstored.o

bin/sdstore: obj/sdstore.o
	$(CC) $(CFLAGS) $(INCLUDE) obj/sdstore.o -o bin/sdstore

obj/sdstore.o: src/sdstore.c
	$(CC) $(CFLAGS) $(INCLUDE) -c src/sdstore.c -o obj/sdstore.o

$(shell mkdir -p obj bin bin/sdstore-transformations)

clean:
	rm -R obj bin
