#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "protocol.h"

int map[map_x][map_y];

Player players[4];
int players_no=0;

void
init_map() {
	int i;
	/*tworzymy mapę*/
	for (i = 0; i < map_x; i++) {
		map[i][0] = 1;
		map[i][map_y-1] = 1;
	}
	for (i = 0; i < map_y; i++) {
		map[0][i] = 1;
		map[map_x-1][i] = 1;
	}
}

void
send_msg(int sock, char *format, ...) {
	char msg[msg_len];
	va_list ap;
	va_start(ap, format);
	vsprintf(msg, format, ap);
	va_end(ap);
	write(sock, msg, msg_len);
	printf("%s\n", msg);
}


void
read_client_msg() {
	int i, j, size;
	char msg[msg_len], dir;
	for (i = 0; i < players_no; i++) {
		size = recv(players[i].socket, msg, sizeof(msg), 0);
		if (size != -1) {
			dir = msg[0];
			players[i].dir = dir;
			for (j = 0; j < players_no; j++)
				send_msg(players[j].socket, "CHDIR %d %c %d %d", i, dir, players[i].x, players[i].y);
		}
	}
}

void
update_map() {
	int i,j;
	char msg[50];
	
	for (i = 0; i < players_no; i++) {
		if (players[i].alive == 0)	
			continue;
		switch (players[i].dir) {
			case 'N':
				players[i].y -= 1;
				break;
			case 'S':
				players[i].y += 1;
				break;
			case 'E':
				players[i].x += 1;
				break;
			case 'W':
				players[i].x -= 1;
				break;
		}
		if (map[players[i].x][players[i].y] == 1) {
			players[i].alive = 0;
			for (j = 0; j < players_no; j++)
				send_msg(players[j].socket, "CRASH %d %d %d", i, players[i].x, players[i].y);
		} else
			map[players[i].x][players[i].y] = 1;
	}
} 

void
show_map() {
	int i, j;
	for (i = 0; i < map_y; i++) {
		for (j = 0; j < map_x; j++)
			if (map[j][i] == 0)
				printf("_");
			else
				printf("#");

		printf("\n");
	}
	printf("\n");
}

int
game_over() {
	int i, ano;
	ano = 0;
	for (i = 0; i < players_no; i++) {
		if (players[i].alive)
			ano++;
	}
	if (ano == 0)
		return 1;
	return 0;
}

int
main(int argc, char *argv[]) {
	int socket_desc, client_sock, c;
	struct sockaddr_in server, client;
	int i;
	char msg[200];
	struct timespec slp;
	
	if (argc < 2)
		players_no = 1;
	else
		players_no = atoi(argv[1]);

	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
		eprintf("could not create socket");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port_nr);

	/*bind*/
	if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)	
		eprintf("bind failed");
	printf("bind done\n");


	listen(socket_desc, 3);
	c = sizeof(struct sockaddr_in);
	
	init_map();
	
	for (i = 0; i < players_no; i++) {
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		fcntl(client_sock, F_SETFL, O_NONBLOCK);
		init_player(i, client_sock);
		send_msg(client_sock, "ID %d %d", i, players_no);
	}
	
	for (i = 0; i < players_no; i++)
		send_msg(players[i].socket, "START");
	
	for (;;) {
		read_client_msg();
		update_map();
		if (game_over())
			break;
		//show_map();
		slp.tv_sec = 0;
		slp.tv_nsec = tick_ms*1000000;
		nanosleep(&slp, NULL);
	}
	
	/*zamykamy gniazda*/
	for (i = 0; i < players_no; i++)
		close(players[i].socket);
	close(socket_desc);
	return 0;
}
