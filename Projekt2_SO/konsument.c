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


    // creating key
    key_t key;
    if (!(key = ftok(".", 'B')))
    {
        printf("KNS: Key creating error");
        exit(-2);
    }

    semafor = semget(key, 2, 0600 | IPC_CREAT);

    if (semafor == -1)
    {
        printf("KNS: Error with creating semafor \n");
        exit(-1);
    }
    else
        printf("KNS: Semafor connected, id = %d  key = %d\n", semafor, key);


    // creating key
    key_t memory_key;
    memory_key = ftok(".", 'A');

    char* adress;
    int memory;
    opusc_semafor(1);
    memory = shmget(memory_key, 0, IPC_CREAT | 0600);
    if (memory == -1)
    {
        printf("KNS: Memory key creating error \n");
        exit(-1);
    }
    else
        printf("KNS: Memory has been created %d\n", memory);


    adress = shmat(memory, 0, 0);
    if (*adress == -1)
    {
        printf("KNS: Error with adress port\n");
        shmctl(memory, IPC_RMID, 0);
        return -1;
    }
    else
    {
        printf("KNS: The address port has been granted\n");
    }
    podnies_semafor(0);





    char znak;
    // open file wyjscie
    FILE* wyjscie;
    wyjscie = fopen("wyjscie", "w");
    if (wyjscie == NULL) {
        printf("KNS: Error with opening file \n");
        //clearing memory_adress
        shmdt(adress);

        //removing memory segment
        shmctl(memory, IPC_RMID, 0);

        if ((semctl(semafor, 0, IPC_RMID)) == -1)
        {
            printf("KNS: Unreal to remove semafor\n");
            exit(-1);
        }
        else
            printf("KNS: Semafor deleted: %d\n", semafor);
        exit(1);
    }


        do
        {
            opusc_semafor(1); // 1 - 0
            znak = *adress;
            if (znak != EOF)
            {
                fputc(znak, wyjscie);
                printf("KNS: Znak write = %c\n", znak);
            }
            podnies_semafor(0); // 0 - 1
        } while (znak != EOF);





    //closing file wyjscie
    if (fclose(wyjscie) == EOF)
    {
        printf("KNS: Error with closing file\n");
        exit(-1);
    }
    //clearing memory_adress
    shmdt(adress);

    //removing memory segment
    shmctl(memory, IPC_RMID, 0);

    printf("KNS: Konsument end working \n");

   // deleting semafor
    if ((semctl(semafor, 0, IPC_RMID)) == -1)
    {
        printf("KNS: Unreal to remove semafor\n");
        exit(-1);
    }
    else
        printf("Semafor deleted: %d\n", semafor);


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
            perror("KNS: Nie moglem zamknac semafora");
            exit(EXIT_FAILURE);
        }

    }
    else
    {
    printf("KNS: Semafor %d zostal opuszczony.\n", sem);
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
        perror("KNS: Problem with exiting semafor\n");
        exit(-5);
    }
    else
    {
        printf("KNS: Semafor %d zostal podnieszony.\n", sem);
    }
}

static void ustaw_semafor(int sem, int nr)
{
    int ustaw_sem;
    ustaw_sem = semctl(semafor, sem, SETVAL, nr);

    if (ustaw_sem == -1)
    {
        printf("KNS: Nie mozna ustawic semafora.\n");
        exit(-1);
    }
    else
    {
        printf("KNS: Semafor zostal ustawiony.\n");
    }
}

