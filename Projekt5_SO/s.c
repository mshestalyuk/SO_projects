#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <ctype.h>
#include <stdbool.h>

#define NAZWA_FIFO "./SERWER_fifo"
#define MAX 64

typedef struct MESSAGE
{
    pid_t clientPID;
    char messageText[MAX];
} message;

void exit_code(int signal);

int main()
{
    if (access(NAZWA_FIFO, F_OK) == -1) // F_OK czy istnieje file
    {
        if (mkfifo(NAZWA_FIFO, 0600) != 0)
        {
            perror("SERWER: fifo creation error");
            exit(EXIT_FAILURE);
        }
        printf("Serwer stworzyl fifo: %s\n", NAZWA_FIFO);
    }
    else
    {
        printf("SERWER: Proba usuniecia istniajacej kolejki\n");
        if (unlink(NAZWA_FIFO) == -1)
        {
            perror("SERWER: fifo removing error");
        }

        if (access(NAZWA_FIFO, F_OK) == -1)
        {
            if (mkfifo(NAZWA_FIFO, 0402) != 0)
            {
                perror("SERWER: fifo creation error");
                exit(EXIT_FAILURE);
            }
            printf("SERWER: Istniajaca kolejka zostala usunieta\n");
            printf("\nSerwer utworzyl fifo: %s\n", NAZWA_FIFO);
        }
        else
        {
            printf("Nie udalo sie usunac istniajaca kolejke\n");
            exit(EXIT_FAILURE);
        }
    }




    signal(SIGINT, exit_code);

    while (true)
    {
        message currentMessage;

        int serwerFifoDeskriptor = open(NAZWA_FIFO, O_RDONLY);
        if (serwerFifoDeskriptor == -1)
        {
            perror("SERWER: qeue opening error");
            exit(EXIT_FAILURE);
        }




        int read_result = read(serwerFifoDeskriptor, &currentMessage, sizeof(currentMessage));
        if (read_result == -1)
        {
            perror("SERWER: fifo read error");
            exit(EXIT_FAILURE);
        }
        if (read_result == 0)
        {
            printf("\nSerwer konczy prace\n");

            if (close(serwerFifoDeskriptor) == -1)
            {
                perror("SERWER: serwer qeue closing error");
                exit(EXIT_FAILURE);
            }

            if (unlink(NAZWA_FIFO) == -1)
            {
                perror("SERWER: fifo removing error");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }


        if (close(serwerFifoDeskriptor) == -1)
        {
            perror("SERWER: serwer qeue closing error");
        }




        printf("Odczytano: %s\n", currentMessage.messageText);

        char klientFifoName[25];
        sprintf(klientFifoName, "./klient_%d", currentMessage.clientPID);

        int messageSize = strlen(currentMessage.messageText);
        for (int i = 0; i < messageSize; i++)
        {
            currentMessage.messageText[i] = toupper(currentMessage.messageText[i]);
        }


        int klientFifoDeskriptor = open(klientFifoName, O_WRONLY);
        if (serwerFifoDeskriptor == -1)
        {
            perror("SERWER: klient qeue opening error");
            exit(EXIT_FAILURE);
        }


        int write_res = write(klientFifoDeskriptor, &currentMessage, sizeof(currentMessage));
        if (write_res == -1)
        {
            perror("SERWER: klient queue writing error");
            exit(EXIT_FAILURE);
        }

        if (close(klientFifoDeskriptor) == -1)
        {
            perror("SERWER: klient qeue closing error");
        }
    }
}

void exit_code(int signal)
{
    if (signal == SIGINT)
    {
        printf("Serwer konczy prace\n");

        if (unlink(NAZWA_FIFO) == -1)
        {
            perror("SERWER: fifo removing error");
        }
        exit(0);
    }
}

