#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
int i, mainpid = getpid();
char cmd[20];

sprintf(cmd, "pstree -ph %d", mainpid);
printf("Proces macierzysty #1 - UID: %d, GID: %d, PID: %d, PPID: %d\n\n ", getuid(), getgid(), getpid(), getppid());
for(i=1; i<=3; i++)
{
        switch(fork())
        {
                case -1:
                perror("fork error");
                exit(1);
                case 0:
                printf("Proces potomny #%d - UID: %d, GID: %d, PID: %d, PPID: %d\n\n",i, getuid(), getgid(), getpid(), getppid());
                break;
                default:
                printf("Proces macierzysty #%d\n", i);		
        }
        system(cmd);
        sleep(1);
}
return 0;
}

