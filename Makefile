
all: client server server2

server : server.o
	gcc server.o -o server

server2 : server2.o protocol.o
	gcc server2.o protocol.o -o server2

client : client.o protocol.o
	gcc client.o protocol.o -Wall -lm -o client `sdl2-config --cflags --libs`

server.o : server.c
	gcc -c server.c

server2.o : server2.c
	gcc -ggdb3 -Wall -c server2.c

protocol.o : protocol.c
	gcc -ggdb3 -Wall -c protocol.c

client.o : client.c
	gcc -ggdb3 -lm -Wall -c client.c `sdl2-config --cflags --libs`
	
clean:
	rm *.o client server server2
