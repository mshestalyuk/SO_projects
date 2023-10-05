#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/shm.h>

#define SP 0

int semafor, memory, disconnect1, disconnect2;
long* adres;
key_t key;
static void utworz_nowy_semafor();
static void semafor_p(int semNumber);
static void semafor_v(int semNumber);
static void ustaw_semafor(int semNumber);
static void usun_semafor(void);
static void create_memory();
static void update_memory();
void disconnect_memory();

int main() {
    char znak;
    utworz_nowy_semafor();
    create_memory();
    update_memory();
    int pid = getpid();
    srand(time(NULL) + getpid());
    while (1) {
        semafor_p(SP);
        znak = (rand() % 25) + 65;
        adres[2] = znak;
        printf("Writer: PID %d, enter symbol %c.\n", pid, znak);
        semafor_v(SP);
    }
    disconnect_memory();
    return 0;
}

static void utworz_nowy_semafor(void)
{
    if ((key = ftok(".", 'A')) == -1)
    {
        printf("Blad ftok (A)\n");
        exit(2);
    }
    semafor = semget(key, 2, 0200 | IPC_CREAT);
    if (semafor == -1)
    {
        printf("Nie moglem utworzyc nowego semafora.\n");
        exit(EXIT_FAILURE);
    }
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
            exit(EXIT_FAILURE);
        }
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
        exit(EXIT_FAILURE);
    }
}

static void create_memory()
{
    if ((key = ftok(".", 'A')) == -1)
    {
        printf("Blad ftok (A)\n");
        exit(2);
    }
    memory = shmget(key, 2 * sizeof(long int) + 1, 0600 | IPC_CREAT);
    if (memory == -1)
    {
        printf("Problemy z utworzeniem memoryi dzielonej.\n");
        exit(EXIT_FAILURE);
    }
}

static void update_memory()
{
    adres = shmat(memory, 0, 0);
    if (*adres == -1)
    {
        printf("Problem z przydzieleniem adresu.\n");
        exit(EXIT_FAILURE);
    }
}

void disconnect_memory()
{
    disconnect1 = shmctl(memory, IPC_RMID, 0);
    disconnect2 = shmdt(adres);
    if (disconnect1 == -1 || disconnect2 == -1)
    {
        printf("Problemy z odlaczeniem memoryi dzielonej.\n");
        exit(EXIT_FAILURE);
    }
    else printf("memory dzielona zostala odlaczona.\n");
}


