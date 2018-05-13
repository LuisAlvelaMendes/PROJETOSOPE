all: client.o server.o
client.o: client.c client.h
	gcc -c -Wall client.o client.c
server.o: server.h server.c
	gcc -c -Wall -lpthread server.o server.c
