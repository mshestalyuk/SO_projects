#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>

struct bufor {
    int mtype;
    int mvalue;
};
#define MAX2 12
#define MAX 10
#define PELNY 2
#define PUSTY 1
#define zapis pam[MAX+1]
#define odczyt pam[MAX]

int semafor;
int* pam;
static void semafor_p(int semNumber);
static void semafor_v(int semNumber);
int main()
{
        key_t m_key, memory_key, sem_key;
        int msgID, shmID;
        struct bufor komunikat;
        sleep(1);
        //  printf("konsument--------------------------------\n");
        // 
        // KK
        if ((m_key = ftok(".", 'A')) == -1)
        {
            printf("Blad ftok (A)\n");
            exit(2);
        }
        msgID = msgget(m_key, IPC_CREAT | 0666);
        if (msgID == -1)
        {
            printf("blad klejki komunikatow\n");
            exit(1);
        }


        // semafors
        sem_key = ftok(".", 'C');
        if ((semafor = semget(sem_key, 1, IPC_CREAT | 0666)) == -1)
        {
            perror("błąd semafora "); exit(1);
        }
        else
        {
           // printf("KNS: Semafor connected, id = %d  key = %d\n", semafor, sem_key);
        }
        
        //memory
        memory_key = ftok(".", 'B');
        shmID = shmget(memory_key, MAX2 * sizeof(int), IPC_CREAT | 0666);
        if (shmID == -1) { printf("blad pamięci dzielonej\n"); exit(1); }
        pam = (int*)shmat(shmID, NULL, 0);
        if (*pam == -1)
        {
            printf("KNS: Error with adress port\n");
            shmctl(pam, IPC_RMID, 0);
            return -1;
        }


        if (msgrcv(msgID, &komunikat, sizeof(komunikat.mvalue), 2, 0) == -1)
        {
            perror("Blad odbioru komunikatu pelnego");
        }

        semafor_p(0);
        int odczytane = pam[odczyt];
        printf("K_%d - odczyt z bufora %d: %d\n", getpid(), odczyt, odczytane);
        odczyt = (odczyt + 1) % MAX;
        semafor_v(0);



        komunikat.mtype = 1;
        if (msgsnd(msgID, &komunikat, sizeof(komunikat.mvalue), 0) == -1)
        {
            perror("Blad wysylania komunikatu pustego");
        }
        strncpy(komunikat.mvalue, 0, sizeof(komunikat.mvalue));
}

static void semafor_p(int semNumber)
{
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num = semNumber;
    bufor_sem.sem_op = -1;
    bufor_sem.sem_flg = 0;
    zmien_sem = semop(semafor, &bufor_sem, 1);
    if (zmien_sem == -1)
    {
        if (errno == EINTR) {
            semafor_p(semNumber);
        }
        else
        {
            printf("Nie moglem zamknac semafora.\n");
            perror("");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        //printf("Semafor zostal zamkniety.\n");
    }
}

static void semafor_v(int semNumber)
{
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num = semNumber;
    bufor_sem.sem_op = 1;
    bufor_sem.sem_flg = 0;
    zmien_sem = semop(semafor, &bufor_sem, 1);
    if (zmien_sem == -1)
    {
        printf("Nie moglem otworzyc semafora.\n");
        perror("");
        exit(EXIT_FAILURE);
    }
    else
    {
      //  printf("Semafor zostal otwarty.\n");
    }
}
