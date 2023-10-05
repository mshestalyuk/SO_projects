#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdbool.h>

#define NAZWA_FIFO "./SERWER_fifo"
#define MAX 64
#define ITERATIONS_NUMBER 4
typedef struct MESSAGE
{
    pid_t clientPID;
    char messageText[MAX];
} message;

int main()
{
    char klientFifoName[25];
    sprintf(klientFifoName, "./klient_%d", getpid());
    printf("kolejka klienta: %s\n", klientFifoName);

    if (access(klientFifoName, F_OK) == -1)
    {
        if (mkfifo(klientFifoName, 0606) != 0)
        {
            perror("KLIENT: fifo creation error");
            exit(EXIT_FAILURE);
        }
        printf("Klient stworzyl fifo: %s\n", klientFifoName);
    }
    else
    {
        printf("KLIENT: queue creating error: queue already exist\n");
        exit(EXIT_FAILURE);
    }

    if (access(NAZWA_FIFO, F_OK) == -1)
    {
        perror("KLIENT: serwer queue does not exist error");
        if (unlink(klientFifoName) == -1)
        {
            perror("KLIENT: fifo removing error");
        }
        exit(EXIT_FAILURE);
    }


    int serwerFifoDeskriptor;

    for (int i = 0; i < ITERATIONS_NUMBER; i++)
    {

        serwerFifoDeskriptor = open(NAZWA_FIFO, O_WRONLY);
        if (serwerFifoDeskriptor == -1)
        {
            perror("KLIENT: serwer qeue opening error");
            exit(EXIT_FAILURE);
        }


        message currentMessage;
        printf("CLIENT: Enter message: ");
        fgets(currentMessage.messageText, MAX, stdin);
        if (strlen(currentMessage.messageText) == MAX - 1)
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) ;
        }

        currentMessage.clientPID = getpid();

        int write_res = write(serwerFifoDeskriptor, &currentMessage, sizeof(currentMessage));
        if (write_res == -1)
        {
            perror("KLIENT: klient queue writing error");
            exit(EXIT_FAILURE);
        }

        int klientFifoDeskriptor = open(klientFifoName, O_RDONLY);
        if (serwerFifoDeskriptor == -1)
        {
            perror("KLIENT: klient qeue opening error");
            exit(EXIT_FAILURE);
        }


        int read_result = read(klientFifoDeskriptor, &currentMessage, sizeof(currentMessage));

        if (read_result == -1)
        {
            perror("KLIENT: fifo read error");
            exit(EXIT_FAILURE);
        }


        if (close(klientFifoDeskriptor) == -1)
        {
            perror("KLIENT: klient qeue closing error");
        }


        printf("Klient otrzymal od serwera: %s\n", currentMessage.messageText);
    }


    if (close(serwerFifoDeskriptor) == -1)
    {
        perror("KLIENT: serwer qeue closing error");
    }


    printf("Klient %d konczy prace\n", getpid());

    if (unlink(klientFifoName) == -1)
    {
        perror("KLIENT: fifo removing error");
    }

    exit(0);
}
