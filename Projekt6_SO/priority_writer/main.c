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
#define SC 1
#define W1 2
#define W2 3
#define W3 4
int semafor, memory, disconnect1, disconnect2;
long* adres;
key_t key;
static void utworz_nowy_semafor();
static void ustaw_semafor(int semNumber);
static void usun_semafor(void);

static void create_memory();
static void update_memory();
void disconnect_memory();
void sighandler();
long writers, readers;
int* forki;

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        printf("Niepoprawna liczba argumentow!\n");
        return 1;
    }
    char* end;
    writers = strtol(argv[1], &end, 10);
    if (*end != '\0' || writers > INT_MAX || writers < INT_MIN) {
        perror("blad konwersji.\n");
        exit(EXIT_FAILURE);
    }
    readers = strtol(argv[2], &end, 10);
    if (*end != '\0' || readers > INT_MAX || readers < INT_MIN) {
        perror("blad konwersji.\n");
        exit(EXIT_FAILURE);
    }
    long places = strtol(argv[3], &end, 10);
    if (*end != '\0' || places > INT_MAX || places < INT_MIN) {
        perror("blad konwersji.\n");
        exit(EXIT_FAILURE);
    }
    if (writers < 1 || readers < 1 || places < 1) {
        printf("Podano bledne wartosci.\n");
        exit(EXIT_FAILURE);
    }
    forki = malloc((writers + readers) * sizeof(int));

    char ulimitSize[7];
    FILE* ulimitFile = popen("ulimit -p", "r");
    if (ulimitFile == NULL)
    {
        perror("Blad przy wywolaniu polecenia ulimit -p");
        exit(EXIT_FAILURE);
    }
    fgets(ulimitSize, sizeof(ulimitSize), ulimitFile);
    long ulimit = strtol(ulimitSize, NULL, 10);
    if (errno != 0) {
        perror("strtol");
        exit(EXIT_FAILURE);
    }
    if (pclose(ulimitFile) == -1) {
        perror("pclose ulimit error.\n");
        exit(EXIT_FAILURE);
    }

    char processesSize[4];
    FILE* processesFile = popen("ps -ux | wc -l", "r");
    if (processesFile == NULL)
    {
        perror("Blad przy wywolaniu polecenia 'ps -ux | wc -l'");
        exit(EXIT_FAILURE);
    }
    fgets(processesSize, sizeof(processesSize), processesFile);
    if (pclose(processesFile) == -1) {
        perror("pclose ps -ux | wc -l error.\n");
        exit(EXIT_FAILURE);
    }
    long pslimit = strtol(processesSize, NULL, 10);
    if (errno != 0) {
        perror("strtol");
        exit(EXIT_FAILURE);
    }
    long limit = ulimit - pslimit + 4;
    if (limit < writers + readers)
    {
        fprintf(stderr, "Przekroczono limit(%ld).\n", limit);
        exit(EXIT_FAILURE);

    }
    create_memory();
    update_memory();
    adres[0] = places;
    adres[1] = 0; //liczba procesow czytajacych
    adres[2] = 0; //liczba procesow piszacych
    adres[3] = ' ';
    utworz_nowy_semafor();
    ustaw_semafor(SP);
    ustaw_semafor(SC);
    ustaw_semafor(W1);
    ustaw_semafor(W2);
    ustaw_semafor(W3);
    semctl(semafor, 5, SETVAL, 0);
    signal(SIGINT, sighandler);
    for (int i = 0; i < writers; i++) {
        switch (forki[i] = fork()) {
        case -1:
            perror("Fork error");
            exit(1);
        case 0:
            if ((execl("./w", "w", NULL)) == -1)
            {
                perror("Execl error");
                exit(2);
            };
        default:
            break;
        }
    }

    for (int i = 0; i < readers; i++) {
        switch (forki[writers + i] = fork()) {
        case -1:
            perror("Fork error");
            exit(1);
        case 0:
            if ((execl("./r", "r", NULL)) == -1)
            {
                perror("Execl error");
                exit(2);
            };
        default:
            break;
        }
    }
    int status;
    for (int i = 0; i < writers + readers; i++) {
        if (wait(&status) == -1) {
            perror("wait function error.\n");
            sighandler();
            exit(EXIT_FAILURE);
        }
    }

    disconnect_memory();
    usun_semafor();
    return 0;
}

void sighandler() {
    for (int i = 0; i < writers + readers; i++)
    {
        if (forki[i] != -1)
        {
            kill(forki[i], SIGINT);
        }
    }
    disconnect_memory();
    usun_semafor();
    int status;
    while (wait(&status) > 0);
    exit(0);
}

static void utworz_nowy_semafor(void)
{
    if ((key = ftok(".", 'A')) == -1)
    {
        printf("Blad ftok (A)\n");
        exit(2);
    }
    semafor = semget(key, 6, 0600 | IPC_CREAT);
    if (semafor == -1)
    {
        printf("Nie moglem utworzyc nowego semafora.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Utworzono semafor %d\n", semafor);
    }
}

static void create_memory()
{
    if ((key = ftok(".", 'A')) == -1)
    {
        printf("Blad ftok (A)\n");
        exit(2);
    }
    memory = shmget(key, 3 * sizeof(long int) + 1, 0600 | IPC_CREAT);
    if (memory == -1)
    {
        printf("Problemy z utworzeniem memoryi dzielonej.\n");
        exit(EXIT_FAILURE);
    }
    else printf("memory dzielona zostala utworzona : %d\n", memory);
}

static void update_memory()
{
    adres = shmat(memory, 0, 0);
    if (*adres == -1)
    {
        printf("Problem z przydzieleniem adresu.\n");
        exit(EXIT_FAILURE);
    }
    else printf("Przestrzen adresowa zostala przyznana\n");
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

static void ustaw_semafor(int semNumber)
{
    int ustaw_sem;
    ustaw_sem = semctl(semafor, semNumber, SETVAL, 1);
    if (ustaw_sem == -1)
    {
        printf("Nie mozna ustawic semafora.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Semafor zostal ustawiony.\n");
    }
}



static void usun_semafor(void)
{
    int sem;
    sem = semctl(semafor, 0, IPC_RMID);

    if (sem == -1)
    {
        printf("Nie mozna usunac semafora.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Usunieto semafor: %d\n", semafor);
    }
}


