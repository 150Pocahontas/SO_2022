### Makefile ###

CC = gcc
CFLAGS = -Wall -g -fno-stack-protector
INCLUDE = -I src/include

all : fifo server client transformations move

fifo: src/mkfifo.c
	gcc -g src/mkfifo.c -o bin/fifo
	./bin/fifo

transformations:
	make -C SDStore-transf

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

move:
	$(shell mv SDStore-transf/nop bin/sdstore-transformations)
	$(shell mv SDStore-transf/gcompress bin/sdstore-transformations)
	$(shell mv SDStore-transf/gdecompress bin/sdstore-transformations)
	$(shell mv SDStore-transf/bcompress bin/sdstore-transformations)
	$(shell mv SDStore-transf/bdecompress bin/sdstore-transformations)
	$(shell mv SDStore-transf/encrypt bin/sdstore-transformations)
	$(shell mv SDStore-transf/decrypt bin/sdstore-transformations)

clean:
	rm -R obj bin
	make clean -C SDStore-transf
	rm fifo_cs fifo_sc pipe_task_done
