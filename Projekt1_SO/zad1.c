#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	printf("PID: %d\n", getpid());	 // process
	printf("PPID: %d\n", getppid()); // parent process
	printf("UID: %d\n", getuid());	 // owner ID
	printf("GID: %d\n\n", getgid()); // group ID
return 0;
}

