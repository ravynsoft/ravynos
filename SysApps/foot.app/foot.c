#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {
	const char *home = getenv("HOME");
	if(home)
		chdir(home);
	int infd = open("/dev/random", O_RDONLY);
	execl("/usr/bin/foot", "foot", "-L", "-W80x25", NULL);
	exit(-1);
}

