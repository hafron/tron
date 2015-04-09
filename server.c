#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/types.h>

void
eprintf(char *e) {
	fprintf(stderr, "%s\n", e);
	exit(1);
}

static int *start;

typedef struct Player Player;
struct Player {
	char nick[100];
	char dir; /*N, S, E, W*/
	int x, y;
	int alive;
	int socket;
};
static int *players_no;
Player *players;



/*
-------> x
|0,0
|
|
|
V
y
*/
enum {map_x = 20, map_y = 50 };
static int map[map_x][map_y];

void
move() {
	int i,j;
	for (i = 0; i < *players_no; i++) {
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
			char msg[50];
			players[i].alive = 0;
			*start = 0;

			sprintf(msg, "CRASH %s %d %d\n", players[i].nick, players[i].x, players[j].y);
			printf("%s", msg);
			for (j = 0; j < *players_no; j++)
				write(players[j].socket, msg, strlen(msg));
		} else
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

void
console() {
	char cmd[100], command[50];
	for (;;) {
		printf("> ");
		fgets(cmd, 100, stdin);
		sscanf(cmd, "%s", command);
		if (strcmp(command, "start") == 0) {
			*start = 1;
			printf("game started\n");
		}
	}
}

int
main(int argc, char *argv[]) {
	int socket_desc, client_sock, c, read_size, clock_pid, pid, cpid, console_pid;
	int i,j;
	struct sockaddr_in server, client;
	char client_message[2000];

	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
		eprintf("could not create socket");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8006 );

	//bind
	if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)	
		eprintf("bind failed");
	printf("bind done\n");


	listen(socket_desc, 3);
	c = sizeof(struct sockaddr_in);

	/*superglobal variables*/
	start = mmap(NULL, sizeof *start, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*start = 0;

	players_no = mmap(NULL, sizeof *players_no, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*players_no = 0;

	players = mmap(NULL, 4*sizeof(Player), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	/*tworzymy mapę*/
	for (i = 0; i < map_x; i++) {
		map[i][0] = 1;
		map[i][map_y-1] = 1;
	}
	for (i = 0; i < map_y; i++) {
		map[0][i] = 1;
		map[map_x-1][i] = 1;
	}


	console_pid = fork();
	if (console_pid == 0) {
		console();
	}

	clock_pid = fork();
	if (clock_pid == 0) {
		for(;;) {
			if (*start) {
				move();
				show_map();
				sleep(1);
			}
		}
	}

	while (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) {
		printf("client conneced\n");
		if (client_sock < 0)
			eprintf("accept failed");

		pid = fork();
		
		if (pid == 0) {
			while ((read_size = recv(client_sock, client_message, 2000, 0)) > 0) {
				char msg[50];
				sscanf(client_message, "%s", msg);
				if (strcmp(msg, "JOIN") == 0) {
					char nick[20];
					sscanf(client_message, "%s %s", msg, nick);
					strcpy(players[*players_no].nick, nick);
					players[*players_no].dir = 'E';
					players[*players_no].x = 10;
					players[*players_no].y = 10;
					players[*players_no].alive = 1;
					players[*players_no].socket = client_sock;
					(*players_no)++;
					printf("joined: %s, players_no: %d\n", nick, *players_no);
				} else {
					strcpy(msg, "unknown command\n");
					write(client_sock, msg, strlen(msg));
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
