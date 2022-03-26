### Makefile ###

CC = gcc -g

all : sdstore sdstored fifo

sdstore: src/sdstore.c
	$(CC) src/sdstore.c $(CFLAGS) -o bin/sdstore

sdstored: sdstored.c
	$(CC) src/sdstored.c $(CFLAGS) -o bin/sdstored

fifo: mkfifo.c
	$(CC) src/mkfifo.c $(CFLAGS) -o bin/mkfifo
	./mkfifo

compile:
	$(CC) src/sdstored.c $(CFLAGS) -o bin/sdstored
	$(CC) src/sdstore.c $(CFLAGS) -o bin/sdstore
	$(CC) src/mkfifo.c $(CFLAGS) -o bin/mkfifo

.PHONY: run
run:
	$(CC) src/sdstored.c $(CFLAGS) -o bin/sdstored
	$(CC) src/sdstore.c  $(CFLAGS) -o bin/sdstore
	./sdstored

.PHONY: clean
clean:
	rm -f sdstored
	rm -f sdstore
	rm -f mkfifo
	rm -f pipe*
	rm -f fifo_sc
	rm -f fifo_cs

.PHONY: exit
exit:
	pkill -f sdstored
