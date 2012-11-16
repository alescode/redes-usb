CC=gcc
CFLAGS= -w

.PHONY: all server client

all: server client

server: server.o date_svc.o
	$(CC) -o server server.o date_svc.o $(CFLAGS)

client: client.o date_clnt.o
	$(CC) -o client client.o date_clnt.o $(CFLAGS)





