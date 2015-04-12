#include <stdio.h>
#include <stdlib.h>

#include<sys/socket.h>
#include<arpa/inet.h>

#include "SDL.h"

#include "protocol.h"

Player players[4];
int players_no=0;

//The window we'll be rendering to
SDL_Window *gWindow = NULL;


//The window renderer
SDL_Renderer* gRenderer = NULL;

void
init_sdl() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		eprintf("SDL could not initialize: %s\n", SDL_GetError());
	
	gWindow = SDL_CreateWindow(	"TRON",\
								SDL_WINDOWPOS_UNDEFINED,\
								SDL_WINDOWPOS_UNDEFINED,\
								map_x, map_y,\
								SDL_WINDOW_SHOWN);
	if (gWindow == NULL)
		eprintf( "window could not be created: %s\n", SDL_GetError());
	
	gRenderer = SDL_CreateRenderer(	gWindow, -1,\
								SDL_RENDERER_ACCELERATED);
	
	if (gRenderer == NULL)
		eprintf("renderer could not be created: %s\n", SDL_GetError());

	//Initialize renderer color
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
}

void
close_sdl()
{
	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	SDL_Quit();
}

typedef struct chain chain;
struct chain {
	int x, y;
	chain *next;
};
chain *chains[4], *chain_tails[4];

void
polygonal_chain(chain *ch) {
	int x, y;
	if (ch == NULL)
		return;
	x = ch->x;
	y = ch->y;
	for (ch = ch->next; ch != NULL; ch = ch->next) {
		SDL_RenderDrawLine(gRenderer, x, y, ch->x, ch->y);
		x = ch->x;
		y = ch->y;
	}
	
}

char address[50];

int
connect_to_server() {
	int sock;
	struct sockaddr_in server;
	char message[1000] , server_reply[2000];
     
	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
		eprintf("Could not create socket");
     
	server.sin_addr.s_addr = inet_addr(address);
	server.sin_family = AF_INET;
	server.sin_port = htons(port_nr);
 
	//Connect to remote server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
		eprintf("connect failed. Error");
	
	return sock;
}

/*może się przepełnić*/
char fifo[100][200];
int nfifo = 0, fifo_id = 0;

int
read_msg(int sock) {
	int i, l;
	char msg[2000], *pmsg, *lsmsg;
	
	if (nfifo > fifo_id) {
		printf("%s\n", fifo[fifo_id]);
		fifo_id++;
		return fifo_id-1;
	}
	
	l = recv(sock, msg, sizeof(msg), 0);
	if (l > 0) {
		pmsg = lsmsg = msg;
		for (i = 0; i < l; i++) {
			if (*pmsg == '\n') {
				*pmsg  = '\0';
				strcpy(fifo[nfifo++], lsmsg);
				lsmsg = pmsg+1;
			}
			pmsg++;
		}
		
		/*wyrzuć najnowszy z kolejki i do boju*/
		fifo_id++;
		return fifo_id-1;
	}
	return -1;
}

int
main(int argc, char *argv[]) {
	int quit, i, sock;
	chain *ch;
	int id;
	
	if (argc > 1)
		strcpy(address, argv[1]);
	else
		strcpy(address, "127.0.0.1");
	
	sock = connect_to_server();
	read_msg(sock);
	
	players_no = 1;
	players[0].dir = 'E';
	players[0].x = 10;
	players[0].y = 10;
	players[0].alive = 1;
	
	for (i = 0; i < players_no; i++) {
		ch = emalloc(sizeof(chain));
		ch->x = players[0].x;
		ch->y = players[0].y;
		chains[i] = ch;
		
		ch = emalloc(sizeof(chain));
		ch->x = players[0].x;
		ch->y = players[0].y;
		ch->next = NULL;
		
		chains[i]->next = ch;
		chain_tails[i] = ch;
	}
	

	init_sdl();
	quit = 0;
	while (!quit) {
		SDL_Event e;
		while (SDL_PollEvent(&e))
			if (e.type == SDL_QUIT)
				quit = 1;
 			//User presses a key
			else if (e.type == SDL_KEYDOWN) {
				int chdir = 0;
				//Select surfaces based on key press
				switch (e.key.keysym.sym) {
				case SDLK_UP:
					if (players[id].dir == 'N')
						break;
					chdir = 'N';
					break;

				case SDLK_DOWN:
					if (players[id].dir == 'S')
						break;
					chdir = 'S';
					break;

				case SDLK_LEFT:
					if (players[id].dir == 'W')
						break;
					chdir = 'W';
					break;

				case SDLK_RIGHT:
					if (players[id].dir == 'E')
						break;
					chdir = 'E';
					break;
                        		}
                        		
                        		if (chdir != 0) {
                        			players[id].dir = chdir;
					ch = emalloc(sizeof(chain));
					ch->x = chain_tails[id]->x;
					ch->y = chain_tails[id]->y;
					
					chain_tails[id]->next = ch;
					chain_tails[id] = ch;
                        		}
                        	}
                        	
                   /*sprawdzamy czy nie nadszedł jakiś komunikat*/
                   
                   /*koniec*/
  
		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);
		
		/*rysujemy krzywe graczy*/
		for (i = 0; i < players_no; i++) {
			if (players[i].alive)
			switch(i) {
				case 0:
					SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
					break;
				case 1:
					SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF);
					break;
				case 2:
					SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);
					break;
				case 3:
					SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0xFF, 0xFF);
					break;
			}
			polygonal_chain(chains[i]);
			
			ch = chain_tails[i];
			switch(players[i].dir) {
				case 'N':
					ch->y--;
					break;
				case 'S':
					ch->y++;
					break;
				case 'E':
					ch->x++;
					break;
				case 'W':
					ch->x--;
					break;
			}
		}
		//Update screen
		SDL_RenderPresent(gRenderer);
		SDL_Delay(tick_ms);
	}
	close_sdl();
	return 0;
}
