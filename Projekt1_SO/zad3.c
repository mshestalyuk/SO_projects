#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
void main()
{
        int w,j,i,x,e;
        for(i=1; i<=3; i++)
        {
                switch(fork())
                {
                case -1:
                        perror("fork error");
                        exit(1);
                        break;
                case 0:
                        e = execl("./zad1","zad1",NULL);
                        if(e == -1){
                              perror("execl error");
                                exit(1);
                        }
                        break;
                default:
			sleep(1);
                        break;
                }
        }
        for(j=1;j<=3;j++){
                w=wait(&x);

              if(w == -1){
                   perror("wait error");
                     exit(1);
                }

                printf("Proces potomny %d zakonczyl sie z kodem %d\n",w, x);
                        }

exit(0);
}
