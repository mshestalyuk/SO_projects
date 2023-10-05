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


#define MAX 4
#define SERWER 1



typedef struct message {
    long serwer;
    long client;
    char mtext[MAX];
} data;


int IDkolejki;
pthread_t thread1, thread2; // thread 1 - sending, thread 1 - receiving



void* sending_message();
void* receiving_message();
void CreateQueue();


int main()
{
    CreateQueue();

        if (pthread_create(&thread1, NULL, &sending_message, NULL))
        {
            perror("CLIENT: Error creating thread - 'sending_message'\n");
            exit(1);
        }

        if (pthread_create(&thread2, NULL, &receiving_message, NULL))
        {
            perror("CLIENT: Error creating thread - 'receiving_message'\n");
            exit(1);
        }

        if (pthread_join(thread1, NULL))
        {
            perror("CLIENT: Error joining thread - 'sending_message'\n");
            exit(2);
        }

        if (pthread_join(thread2, NULL))
        {
            perror("CLIENT: Error joining thread - 'receiving_message'\n");
            exit(2);
        }


}



void* sending_message()
{
    struct msqid_ds msg;
    data kom;
    char character, bufor[MAX];

    msgctl(IDkolejki, IPC_STAT, &msg);
    int max = msg.msg_qbytes - 2 * sizeof(long);
    while (1) {
        msgctl(IDkolejki, IPC_STAT, &msg);
        printf("CLIENT: Message count = %ld\n", msg.msg_qnum);

        while (1) {
           int i = 0;
            printf("CLIENT: Enter message: ");

            while ((character = fgetc(stdin)) != '\n') {
                if (i < (MAX - 1)) {
                   bufor[i] = character;
                }
                i++;
            }

            if (i >= (MAX-1)) {
                bufor[(MAX - 1)] = '\0';
            }
            else {
               bufor[i] = '\0';
            }
           // memset(kom.mtext, '\0', MAX);
            strcpy(kom.mtext, bufor);
            msgctl(IDkolejki, IPC_STAT, &msg);
            if (msg.__msg_cbytes > max) {
                printf("CLIENT: The queue is full, the message has not been sent and the sending thread is terminated\n");
                pthread_exit((void*)0);
            }
            kom.serwer = SERWER;
            kom.client = getpid();
            if (msgsnd(IDkolejki, &kom, MAX + sizeof(long), 0) == -1)
            {
                perror("The server has shut down, the message has not been sent\n");
                exit(0);
            }
            printf("CLIENT: Message sent\n");

        }

    }
}


void* receiving_message()
{
    data kom;

    while (1){
		memset(kom.mtext, '\0', MAX);
                if (msgrcv(IDkolejki, &kom, MAX + sizeof(long), getpid(), 0) == -1)
                {
                    perror("CLIENT: Error fetching message from queue\n");
                    exit(EXIT_FAILURE);
                }

                printf("CLIENT: Receiving message: '%s'\n", kom.mtext);
                sleep(1);
     }

}

void CreateQueue() {
    key_t key;
    if (!(key = ftok(".", 'A')))
    {
        perror("CLIENT: Blad tworzenia klucza\n");
        exit(-2);
    }


    if ((IDkolejki = msgget(key, IPC_CREAT | 0606)) == -1)
    {
        perror("CLIENT: Blad tworzenia kolejki komunikat�w.\n");
        exit(EXIT_FAILURE);
    }
    printf("CLIENT: IDkolejki=%d\n", IDkolejki);
}


