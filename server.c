#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

void
eprintf(char *e) {
	fprintf(stderr, "%s\n", e);
	exit(1);
}

int start = 1;

typedef struct Player Player;
struct Player {
	char nick[100];
	char dir; /*N, S, E, W*/
	int x, y;
	int alive;
};
int players_no = 0;
Player players[4];

int death_notify[0];


/*
-------> x
|0,0
|
|
|
V
y
*/
enum {map_x = 20, map_y = 20 };
int map[map_x][map_y];

void
move() {
	int i;
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
		if (map[players[i].x][players[i].y] == 1)
			players[i].alive = 0;
		else
			map[players[i].x][players[i].y] = 1;
	}
}

void
show_map() {
	int i, j;
	for (i = 0; i < map_x; i++) {
		for (j = 0; j < map_y; j++)
			if (map[i][j] == 0)
				printf("_");
			else
				printf("#");

		printf("\n");
	}
	printf("\n");
}

int
main(int argc, char *argv[]) {
	int socket_desc, client_sock, c, read_size, clock_pid, pid, cpid;
	struct sockaddr_in server, client;
	char client_message[2000];

	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
		eprintf("could not create socket");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8000 );

	//bind
	if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)	
		eprintf("bind failed");
	printf("bind done\n");

	listen(socket_desc, 3);
	c = sizeof(struct sockaddr_in);

	clock_pid = fork();
	if (clock_pid == 0) {
		for(;;) {
			if (start) {
				move();
				show_map();
				sleep(1);
			}
		}
	}

	while (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) {
		printf("client conneced");
		if (client_sock < 0)
			eprintf("accept failed");

		pid = fork();
		
		if (pid == 0) {
			cpid = fork();
			if (cpid == 0) {
				int i;
				for(;;)
				for (i = 0; i < players_no; i++) {
					if (players[i].alive == 0 && death_notify[i] == 0) {
						char msg[20];
						sprintf(msg, "CRASH %s %d %d\n", players[i].nick, players[i].x, players[i].y);
						write(client_sock, msg, strlen(msg));
						death_notify[i] = 1;
					}
				}
			} else {
				while ((read_size = recv(client_sock, client_message, 2000, 0)) > 0) {
					char msg[50];
					sscanf(client_message, "%s", msg);
					if (strcmp(msg, "JOIN") == 0) {
						char nick[20];
						sscanf(client_message, "%s %s", msg, nick);
						strcpy(players[players_no].nick, nick);
						players[players_no].dir = 'E';
						players[players_no].x = 10;
						players[players_no].y = 10;
						players[players_no].alive = 1;
						players_no++;
					} else {
						strcpy(msg, "unknown command\n");
						write(client_sock, msg, strlen(msg));
					}
				}

				if (read_size == 0) {
					printf("client disconnected\n");
					fflush(stdout);
				} else if (read_size == -1)
					eprintf("recv failed");
			}
		}
	}

	return 0;
}
