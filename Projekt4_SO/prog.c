#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdbool.h>

#define INT_MAX 2147483647
int producer(int pipeWriteDescriptor, int charNumber);
int consumer(int pipeReadDescriptor);

int main(int argc, char* argv[]) {
        if (argc != 4)
            {
                printf("Not correct list of arguments\n");
                exit(-1);
            }
    char* end;
    long int num_p, num_k, num_s;

    num_k = strtol(argv[1], &end, 10);

    if (end == argv[1])
    {
        printf("Error");
        return 1;
    }
    if (num_k > INT_MAX || num_k <= 0)
    {
        printf("Out of range int or num_k <= 0\n");
        return 1;
    }


    num_p = strtol(argv[2], &end, 10);

                if (end == argv[2])
                {
                    printf("Error");
                    return 1;
                }
                if (num_p > INT_MAX || num_p <= 0)
                {
                    printf("Out of range int or num_p <= 0\n");
                    return 1;
                }


    num_s = strtol(argv[3], &end, 10);

                if (end == argv[3])
                {
                    printf("Error");
                    return 1;
                }
                if (num_s > INT_MAX || num_s <= 0)
                { 
                    printf("Out of range int or num_s <= 0\n");
                    return 1;
                }

    FILE* ulimit, * procesy;


    ulimit = popen("ulimit -a", "r"); 
    procesy = popen("ps | wc -l", "r"); // read count of processes

    char buf1[20], buf2[48], buf3[10];
    for (int i = 0; i < 7; i++)
    {
        fgets(buf2, 47, ulimit);
        // printf("%s\n", buf2);
    }
    fgets(buf1, 20, procesy);
    fgets(buf2, 47, ulimit);
    pclose(ulimit);
    pclose(procesy);
    int k = 0;
    for (int i = 20; i < 28; i++)
    {
        buf3[k] = buf2[i];
        k++;
    }

    int max = atoi(buf3);
    int current = atoi(buf1);
    int all = num_k + num_p + current;

    printf("Processes: %d\nLimit: %d\n", current, max);

    if (all >= max) {
        printf("Too many proccesses want to run\n");
        return 1;
    }

    int myPipe[2];
    if (pipe(myPipe) == -1)
    {
        perror("Pipe error");
        exit(1);
    }

    for (int i = 0; i < num_p; i++)
    {
        switch (fork())
        {
        case -1:
        {
            perror("Fork error");
            return 1;
            break;
        }
        case 0:
        {
            close(myPipe[0]);
            int result = producer(myPipe[1], num_s);
            return result;
            break;
        }
        default:
        {
            break;
        }
        }
    }

    for (int i = 0; i < num_k; i++)
    {
        switch (fork())
        {
        case -1:
        {
            perror("Fork error");
            return 1;
            break;
        }
        case 0:
        {
            close(myPipe[1]);
            int result = consumer(myPipe[0]);
            return result;
            break;
        }
        default:
        {
            break;
        }
        }
    }

    close(myPipe[1]); // zamykanie deskryptorow macirzystego
    close(myPipe[0]);  // ponizej wywolania potomnych dlatego ze deskptory dziedzicza

    for (int i = 0; i < num_k + num_p; ++i)
        wait(NULL); // block parent proccess until children works

    return 0;

}


int consumer(int pipeReadDescriptor)
{
    int pid = getpid();
    // printf("Konsument %d zaczal prace\n", pid);
    char buf[20], filename[20] = "temp/wy";
    sprintf(buf, "%d", pid);
    strcat(filename, buf);
    FILE* fileOut;
    if ((fileOut = fopen(filename, "w")) == NULL)
    {
        printf("KONSUMENT %d: Nie moge otworzyc pliku do zapisu!\n", pid);
        return 1;
    }

    while (true)
    {
        char tmpChar;
        int readedChars = read(pipeReadDescriptor, &tmpChar, sizeof(tmpChar));
        if (readedChars == 0)
        {
           // printf("CONSUMENT %d: pusta rura\n", pid);
            break;
        }
        if (readedChars == -1)
        {
            printf("CONSUMENT %d: reading error\n", pid);
            return 1;
        }
      //  printf("Consument: get %c\n", tmpChar);
        putc(tmpChar, fileOut);
    }
    // printf("\n");
    close(pipeReadDescriptor);
    // printf("Konsument %d skonczyl prace\n", pid);
    return 0;
}

int producer(int pipeWriteDescriptor, int num_s)
{
    int pid = getpid();
    // printf("Producent %d zaczal prace\n", pid);
    char filename[20] = "temp/we";
        char buf[20];
    sprintf(buf, "%d", pid);
    strcat(filename, buf);    
    FILE* fileOut;
    if ((fileOut = fopen(filename, "w")) == NULL)
    {
        printf("PRODUCENT %d: Nie moge otworzyc pliku do zapisu!\n", pid);
        return 1;
    }
    for (int i = 0; i < num_s; i++)
    {
        char c = rand() % (25) + 97;
      //  printf("%c", c);
        fputc(c, fileOut);
        if (write(pipeWriteDescriptor, &c, 1) <= 0)
        {
            printf("PRODUCENT %d: blad zapisu do rury\n", pid);
            return 1;
        }
    }

    // printf("\n");
    // printf("Producent %d skonczyl prace\n", pid);
    close(pipeWriteDescriptor);
    return 0;
}
