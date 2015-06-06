#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>

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
		/*pogrubiamy linie*/
		/*linia pionowa*/
		if (x == ch->x) {
			SDL_RenderDrawLine(gRenderer, x-1, y, ch->x-1, ch->y);
			SDL_RenderDrawLine(gRenderer, x+1, y, ch->x+1, ch->y);
		} else {
			SDL_RenderDrawLine(gRenderer, x, y-1, ch->x, ch->y-1);
			SDL_RenderDrawLine(gRenderer, x, y+1, ch->x, ch->y+1);
		}
		x = ch->x;
		y = ch->y;
	}
	
}

char address[50];

int
connect_to_server() {
	int sock;
	struct sockaddr_in server;
     
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

void
send_msg(int sock, char *format, ...) {
	char msg[msg_len];
	va_list ap;
	va_start(ap, format);
	vsprintf(msg, format, ap);
	va_end(ap);
	send(sock, msg, msg_len, 0);
	printf("%s\n", msg);
}

int
main(int argc, char *argv[]) {
	int quit, i, sock;
	chain *ch;
	char msg[msg_len];
	int id;
	
	if (argc > 1)
		strcpy(address, argv[1]);
	else
		strcpy(address, "127.0.0.1");
	
	sock = connect_to_server();
	
	recv(sock, msg, msg_len, 0);
	sscanf(msg, "ID %d %d", &id, &players_no);
	printf("your id: %d\n", id);

	for (i = 0; i < players_no; i++) {
		init_player(i, -1);
		
		ch = emalloc(sizeof(chain));
		ch->x = players[i].x;
		ch->y = players[i].y;
		chains[i] = ch;
		
		ch = emalloc(sizeof(chain));
		ch->x = players[i].x;
		ch->y = players[i].y;
		ch->next = NULL;
		
		chains[i]->next = ch;
		chain_tails[i] = ch;
	}
	

	init_sdl();
	quit = 0;
		
	/*oczekiwanie na start*/
	recv(sock, msg, msg_len, 0);
	printf("-> %s\n", msg);
	
	/*przechodzimy w tryb nieblokujący*/
	fcntl(sock, F_SETFL, O_NONBLOCK);
	
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
                        		
                        		if (chdir == 0 || players[id].dir == chdir)
                        			continue;
                        		/*nie zmieniamy kierunku na "wsteczny"*/
                        		if (players[id].dir == 'E' && chdir == 'W')
                        			continue;
                        		if (players[id].dir == 'W' && chdir == 'E')
                        			continue;
                        		if (players[id].dir == 'N' && chdir == 'S')
                        			continue;
                        		if (players[id].dir == 'S' && chdir == 'N')
                        			continue;
                        		
                        		send_msg(sock, "%c", chdir);
                        	}
                        	
                   /*sprawdzamy czy nie nadszedł jakiś komunikat*/
                   if (recv(sock, msg, msg_len, 0) > 0) {
                   	printf("-> %s\n", msg);
                   	char com[20];
                   	sscanf(msg, "%s", com);
                   	if (strcmp(com, "CHDIR") == 0) {
                   		int ch_id, ch_x, ch_y;
                   		char ch_dir;
                   		sscanf(msg, "%s %d %c %d %d", com, &ch_id, &ch_dir, &ch_x, &ch_y);
				players[ch_id].dir = ch_dir;
				/*nanosimy poprawkę*/
				chain_tails[ch_id]->x = ch_x;
				chain_tails[ch_id]->y = ch_y;
				ch = emalloc(sizeof(chain));
				ch->x = ch_x;
				ch->y = ch_y;
				ch->next = NULL;
					
				chain_tails[ch_id]->next = ch;
				chain_tails[ch_id] = ch;
                   	} else if (strcmp(com, "CRASH") == 0) {
                   		int cr_id, cr_x, cr_y;
                   		sscanf(msg, "%s %d %d %d", com, &cr_id, &cr_x, &cr_y);
                   		players[cr_id].alive = 0;
                   		chain_tails[cr_id]->x = cr_x;
                   		chain_tails[cr_id]->y = cr_y;
                   	}
                   }
                   /*koniec*/
  
		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);
		
		/*rysujemy krzywe graczy*/
		for (i = 0; i < players_no; i++) {
			
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
			/*narysuj w rogu kolor gracza*/
			if (i == id) {
				SDL_Rect r;
				r.x = 0;
				r.y = 0;
				r.w = 10;
				r.h = 10;
				SDL_RenderFillRect(gRenderer, &r);
			}
			if (!players[i].alive)
				continue;
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
