/*
-------> x
|0,0
|
|
|
V
y
*/
enum {map_x = 300, map_y = 300 };
enum {msg_len = 200};

typedef struct Player Player;
struct Player {
	char dir; /*N, S, E, W*/
	int x, y;
	int alive;
	int socket;
};

static int tick_ms=20;
static int margin=10;

static int port_nr = 8001;

extern int players_no;
extern Player players[4];

void eprintf(char *format, ...);
void * emalloc(int size);
void init_player(int i, int sock);