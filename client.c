#include <stdio.h>

int
main(int argc, char *argv[]) {
	char command[100];
	for(;;){
		fgets(command, sizeof(command), stdin);
		printf("%s", command);
	}	
	return 0;
}
