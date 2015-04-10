#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <arpa/inet.h>

void
eprintf(char *e) {
	fprintf(stderr, "%s\n", e);
	exit(1);
}

/*
-------> x
|0,0
|
|
|
V
y
*/
enum {map_x = 50, map_y = 20 };
int map[map_x][map_y];

typedef struct Player Player;
struct Player {
	char dir; /*N, S, E, W*/
	int x, y;
	int alive;
	int socket;
};
int players_no=0;
Player players[4];

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
init_player(int i, int sock) {
	int margin=10;
	players[i].socket = sock;
	players[i].alive = 1;
	switch (i) {
		case 0:
			players[i].x = margin;
			players[i].y = margin;
			players[i].dir = 'E';
			break;
		case 1:
			players[i].x = map_x - margin;
			players[i].y = margin;
			players[i].dir = 'W';
			break;
		case 2:
			players[i].x = map_y -margin;
			players[i].y = map_x - margin;
			players[i].dir = 'E';
			break;
		case 3:
			players[i].x = margin;
			players[i].y = map_y - margin;
			players[i].dir = 'W';
			break;
			
	}
}

void
read_client_msg() {
	int i, j, size;
	char msg[200], dir;
	for (i = 0; i < players_no; i++) {
		size = recv(players[i].socket, msg, sizeof(msg), 0);
		if (size != -1) {
			dir = msg[0];
			players[i].dir = dir;
			for (j = 0; j < players_no; j++) {
				sprintf(msg, "CHDIR %d %c\n", i, dir);
				write(players[j].socket, msg, strlen(msg));
			}
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
			sprintf(msg, "CRASH %d %d %d\n", i, players[i].x, players[i].y);
			printf("%s", msg);
			for (j = 0; j < players_no; j++)
				write(players[j].socket, msg, strlen(msg));
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
	int tick_ms = 200;
	
	if (argc < 2)
		players_no = 1;
	else
		players_no = atoi(argv[1]);

	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
		eprintf("could not create socket");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8000 );

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
		sprintf(msg, "ID %d\n", i);
		write(client_sock, msg, strlen(msg));
	}
	
	sprintf(msg, "START %d %d %d %d\n", map_x, map_y, players_no, tick_ms);
	for (i = 0; i < players_no; i++)
		write(players[i].socket, msg, strlen(msg));
	
	for (;;) {
		read_client_msg();
		update_map();
		if (game_over())
			break;
		show_map();
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
