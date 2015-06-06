#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "protocol.h"

void
eprintf(char *format, ...) {
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
}

void *
emalloc(int size) {
	void *p;
	p = malloc(size);
	if (p == NULL)
		eprintf("malloc: no memory");
	return p;
}

void
init_player(int i, int sock) {
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
			players[i].dir = 'W';
			break;
		case 3:
			players[i].x = margin;
			players[i].y = map_y - margin;
			players[i].dir = 'E';
			break;
			
	}
}