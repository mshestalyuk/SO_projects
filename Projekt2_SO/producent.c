#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/shm.h>

int semafor;
static void opusc_semafor(int);
static void podnies_semafor(int);
static void ustaw_semafor(int sem, int nr);

int main()
{

    key_t key;
    if (!(key = ftok(".", 'B')))
    {
        printf("PR: Key creating error");
        exit(-2);
    }

    semafor = semget(key, 2, 0600 | IPC_CREAT);

    if (semafor == -1)
    {
        printf("PR: Error with creating semafor \n");
        exit(-1);
    }
    else
        printf("Semafor created, id = %d  key = %d\n", semafor, key);


    ustaw_semafor(0, 1);
    ustaw_semafor(1, 0);

    // creating key
    key_t memory_key;
    memory_key = ftok(".", 'A');

    char* adress;
    int memory;
    opusc_semafor(0);
    memory = shmget(memory_key, 1, IPC_CREAT | 0600);
    if (memory == -1)
    {
        printf("PR: Memory key creating error \n");
        exit(-1);
    }
    else
        printf("PR: Memory has been created %d\n", memory);


    adress = shmat(memory, 0, 0);
    if (*adress  == -1)
    {
        printf("PR: Error with adress port\n");
        shmctl(memory, IPC_RMID, 0);
        return -1;
    }
    else
    {
        printf("PR: The address port has been granted\n");
    }
    podnies_semafor(1);


    // creating key

    char znak;
    // open file wyjscie
    FILE* wejscie;
    wejscie = fopen("wejscie", "r");

    if (wejscie == NULL) {
        printf("PR: Error with opening file\n");
        exit(1);
    }
   
        do
        {
            znak = getc(wejscie);
            opusc_semafor(0); // 0 - 0

            *adress = znak;
            if (znak != EOF)
                printf("PR: Znak read = %c\n", znak);

            podnies_semafor(1); // 1 - 1
        } while (!feof(wejscie));
    



    //closing file wyjscie
    if (fclose(wejscie) == EOF)
    {
        printf("PR: Error with closing file\n");
        exit(-1);
    }

    //clearing memory_adress
    shmdt(adress);
    opusc_semafor(1); // 0 - 0

    printf("PR: Producent end working \n");

    return 0;
}

static void opusc_semafor(int sem)
{
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num = sem;
    bufor_sem.sem_op = -1;
    bufor_sem.sem_flg = 0;
    zmien_sem = semop(semafor, &bufor_sem, 1);

    if (zmien_sem == -1)
    {
        if (errno == EINTR)
        {
            opusc_semafor(sem);
        }
        else
        {
            perror("PR: Nie moglem zamknac semafora");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("PR: Semafor %d zostal opuszczony.\n", sem);
    }
}


static void podnies_semafor(int sem)
{
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num = sem;
    bufor_sem.sem_op = 1;
    bufor_sem.sem_flg = 0;
    zmien_sem = semop(semafor, &bufor_sem, 1);

    if (zmien_sem == -1)
    {
        perror("PR: Problem with exiting semafor\n");
        exit(-5);
    }
    else
    {
        printf("PR: Semafor %d zostal podniesony.\n", sem);
    }
}

static void ustaw_semafor(int sem, int nr)
{
    int ustaw_sem;
    ustaw_sem = semctl(semafor, sem, SETVAL, nr);

    if (ustaw_sem == -1)
    {
        printf("PR: Nie mozna ustawic semafora.\n");
        exit(-1);
    }
    else
    {
        printf("PR: Semafor %d zostal ustawiony.\n", sem);
    }
}

