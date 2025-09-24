CFLAGS = -g -Wall
LDFLAGS =
CC = gcc

default: fz35-cli.c
	$(CC) $(CFLAGS) fz35-cli.c fz35-serial.c -o fz35-cli $(LDFLAGS)

clean: 
	rm -f fz35-cli
