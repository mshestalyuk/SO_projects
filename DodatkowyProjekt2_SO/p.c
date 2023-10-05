#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <time.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>


struct bufor {
    int mtype;
    int mvalue;
};


#define MAX 10
#define MAX2 12
#define PELNY 2
#define PUSTY 1
#define odczyt pam[MAX]
#define zapis pam[MAX+1]

int semafor;
int* pam;
static void semafor_p(int semNumber);
static void semafor_v(int semNumber);
int main()
{
    key_t klucz, memory_key, sem_key;
    int msgID;
    int shmID;
    time_t czas;
    struct bufor komunikat;
    if ((klucz = ftok(".", 'A')) == -1)
    {
        printf("Blad ftok (A)\n");
        exit(2);
    };

    msgID = msgget(klucz, IPC_CREAT | 0666);
    
    if (msgID == -1)
    {
        printf("blad klejki komunikatow\n");
        exit(1);
    }

    sem_key = ftok(".", 'C');

    if ((semafor = semget(sem_key, 1, IPC_CREAT |  0666)) == -1)
    {
        perror("błąd semafora "); exit(1);
    }
    else
    {
      //  printf("PR: Semafor connected, id = %d  key = %d\n", semafor, sem_key);
    }

    memory_key = ftok(".", 'B');
    shmID = shmget(memory_key, MAX2 * sizeof(int), IPC_CREAT | 0666);
    pam = (int*)shmat(shmID, NULL, 0);

    //odbi�r komunikatu
        if (msgrcv(msgID, &komunikat, sizeof(komunikat), 1, 0) == -1)
        {
            perror("Blad odbioru komunikatu pustego");
        }
    
        
        sleep(getpid() % 10);



    // w sekcji krytycznej  zapis PID-u (getpid()) do puli bufor�w pod indeks zapis
    //wypisanie informacji na ekranie
    //modyfikacja indeksu do zapisu
    //wys�anie odpowiedniego komunikatu
        semafor_p(0);
        pam[zapis] = getpid();
        printf("P_%d: Zapisalem pod bufor %d: %d\n", getpid(), zapis, pam[zapis]);
        zapis = (zapis + 1) % MAX;
        semafor_v(0);

        komunikat.mtype = 2;
        if (msgsnd(msgID, &komunikat, sizeof(komunikat.mvalue), 0) == -1)
        {
            perror("Blad wysylania komunikatu pelnego");
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
      //  printf("Semafor zostal zamkniety.\n");
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
