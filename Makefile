### Makefile ###

CC = gcc
CFLAGS = -Wall -g -std=c99
INCLUDE = -I src/include

all : fifo server client

fifo: src/mkfifo.c
	gcc -g src/mkfifo.c -o bin/mkfifo
	./bin/mkfifo

server: bin/sdstored

client: bin/sdstore

bin/sdstored: obj/sdstored.o
	$(CC) $(CFLAGS) $(INCLUDE) obj/sdstored.o -o bin/sdstored

obj/sdstored.o: src/sdstored.c
	$(CC) $(CFLAGS) $(INCLUDE) -c src/sdstored.c -o obj/sdstored.o


bin/sdstore: obj/sdstore.o
	$(CC) $(CFLAGS) $(INCLUDE) obj/sdstore.o -o bin/sdstore

obj/sdstore.o: src/sdstore.c
	$(CC) $(CFLAGS) $(INCLUDE) -c src/sdstore.c -o obj/sdstore.o

$(shell mkdir -p obj bin)

clean:
	rm -R obj bin
