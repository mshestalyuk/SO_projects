#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#define MAX 4


#define SERWER 1
int IDkolejki;

void serving_sigint(int sig_n);
void CreateQueue();

typedef struct message {
    long serwer;
    long client;
    char mtext[MAX];
} data;

int main() {
    CreateQueue();

    signal(SIGINT, serving_sigint);
    printf("SERWER: Signal CTRL+C end working of serwer!\n");


    data kom;
    while (1)
    {
        printf("SERWER: Waiting for message ->\n");
        if ((msgrcv(IDkolejki, &kom, MAX+sizeof(long), SERWER, 0)) == -1)
        {
            perror("SERWER: Error fetching message from queue\n");
            exit(EXIT_FAILURE);
        }



        printf("SERWER: Received: \"%s\" zaadresowane do %ld\n", kom.mtext, kom.client);
        kom.serwer = kom.client;
        kom.client = SERWER;

        for (int i = 0; i < strlen(kom.mtext); i++) {
            kom.mtext[i] = toupper(kom.mtext[i]);
        }

        printf("SERWER: Sending '%s' to client %ld\n", kom.mtext, kom.client);
        if ((msgsnd(IDkolejki, &kom, MAX + sizeof(long), 0)) == -1)
        {
            perror("SERWER: Error sending messages\n");
            exit(EXIT_FAILURE);
        }
    }


    if (msgctl(IDkolejki, IPC_RMID, 0) == -1)
    {
        printf("SERWER: Error removing queue\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("SERWER:Queue removed with ID: %d\n", IDkolejki);

    exit(0);
}

void serving_sigint(int sig_n)
{
    key_t key;
    if (!(key = ftok(".", 'A')))
    {
        perror("SERWER: Error with creating key\n");
        exit(EXIT_FAILURE);
    }

    if (sig_n == SIGINT) {
        printf("SERWER: End serwer working\n");
        if ((msgctl(IDkolejki, IPC_RMID, 0)) == -1) {
            printf("SERWER: Error removing queue\n");
            exit(EXIT_FAILURE);
        }
        else
            printf("SERWER: Queue removed with ID: %d\n", IDkolejki);

        exit(0);
    }
}

void CreateQueue() {
    key_t key;
    if (!(key = ftok(".", 'A')))
    {
        perror("SERWER: Blad tworzenia klucza\n");
        exit(-2);
    }


    if ((IDkolejki = msgget(key, IPC_CREAT | 0606)) == -1)
    {
        perror("SERVER: Blad tworzenia kolejki komunikat�w.\n");
        exit(EXIT_FAILURE);
    }
    printf("SERWER: IDkolejki=%d\n", IDkolejki);
}
