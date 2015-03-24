
all: client server

server : server.o
	gcc server.o -o server

client : client.o
	gcc client.o -o client

server.o : server.c
	gcc -c server.c

client.o : client.c
	gcc -c client.c
	
clean:
	rm *.o client server
