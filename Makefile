CFLAGS = -Wall
CC = gcc

all: client server

client : Client.o utile.o
		$(CC) $(CFLAGS) -o client Client.o utile.o

server : Server.o utile.o
		$(CC) $(CFLAGS) -o server Server.o utile.o

utile.o : utile.h

Server.o : utile.h

Client.o : utile.h

clean: 
		rm *.o client server
