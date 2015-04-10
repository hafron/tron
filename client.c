#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"

void
eprintf(char *format, ...) {
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
}

int map_x = 50;
int map_y = 20;

//The window we'll be rendering to
SDL_Window *gWindow = NULL;


//The window renderer
SDL_Renderer* gRenderer = NULL;

void
init() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		eprintf("SDL could not initialize: %s\n", SDL_GetError());
	
	gWindow = SDL_CreateWindow(	"SDL Tutorial",\
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

void close()
{
	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	SDL_Quit();
}

int
main(int argc, char *argv[]) {
	init();
	for (;;) {
		SDL_Event e;
		if (SDL_PollEvent(&e))
			if (e.type == SDL_QUIT) {
   				break;
			}
	}
	close();
	return 0;
}
