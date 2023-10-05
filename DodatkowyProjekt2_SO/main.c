#include <stdio.h>
#include <sys/shm.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <signal.h>
#include <errno.h>

#define P 12  // liczba  procesow producent�w i konsument�w
#define MAX 10 //rozmiar puli bufor�w
#define MAX2 12// rozmiar pami�ci dzielonej
#define PUSTY 1 //typ komunikatu
#define PELNY 2 //typ komunikatu

int semafor;
//struktura komunikatu
struct bufor {
	long mtype;
	int mvalue;
};
int shmID, msgID;  //ID semafora, kolejki kom. pami�ci dzielonej

//funkcja koniec -- do obs�ugi przerwania
void koniec(int sig)
{
	msgctl(msgID, IPC_RMID, NULL);
	shmctl(shmID, IPC_RMID, NULL);
	semctl(semafor, 0, IPC_RMID, NULL);
	printf("MAIN - funkcja koniec sygnal %d: Koniec.\n", sig);
	exit(1);

}

int main()
{
	key_t m_key, memory_key, sem_key;  //m_keye do IPC
	int i;
	struct bufor komunikat;
	struct sigaction act;
	act.sa_handler = koniec;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, 0);

	if ((m_key = ftok(".", 'A')) == -1)							// Tworzenia klucza KK
	{
		printf("Blad ftok (main)\n");
		exit(1);
	}

	msgID = msgget(m_key, IPC_CREAT | IPC_EXCL | 0666);			// Tworzenia KK
	if (msgID == -1)
	{
		printf("blad kolejki komunikatow\n"); exit(1);
	}

	memory_key = ftok(".", 'B');		// Memory key
	if (memory_key == -1)
	{
		printf("blad kolejki pamieci dzielonej\n"); exit(1);
	}

	shmID = shmget(memory_key, MAX2 * sizeof(int), IPC_CREAT | IPC_EXCL | 0666);		// Create memory
	if (shmID == -1) { printf("blad pamięci dzielonej\n"); exit(1); }

	sem_key = ftok(".", 'C');									// sem_key
	if (sem_key == -1)
	{
		printf("blad kolejki pamieci dzielonej\n"); exit(1);
	}

	semafor = semget(sem_key, 1, IPC_CREAT | 0666);					// semafor
   // printf("Main: Semafor connected, id = %d  key = %d\n", semafor, sem_key);
	if (semafor == -1) { printf("blad semaforów \n"); exit(1); }

	if ((semctl(semafor, 0, SETVAL, 1)) == -1)				// wartosc semafora
	{
		perror("");
	}

	komunikat.mtype = PUSTY;							// komunikat
	//   komunikat.mvalue=0;
	for (i = 0; i < MAX; i++)
	{
		if (msgsnd(msgID, &komunikat, sizeof(komunikat.mvalue), 0) == -1)
		{
			printf("blad wyslania kom. pustego\n");
			exit(1);
		};
		printf("wyslany pusty komunikat %d\n", i);
	}

	for (i = 0; i < P; i++)
	{
		switch (fork())
		{
		case -1:
			perror("Blad fork (mainprog)");
			exit(2);
		case 0:
			execl("./prod", "prod", NULL);
		}

		switch (fork())
		{
		case -1:
		printf("Blad fork (mainprog)\n");
		exit(2);
		case 0:
		execl("./kons", "kons", NULL);
		}

		//usleep(500);

	}

	for (i = 0; i < 2 * P; i++)
		wait(NULL);

	msgctl(msgID, IPC_RMID, NULL);
	shmctl(shmID, IPC_RMID, NULL);
	semctl(semafor, 0, IPC_RMID, NULL);
	printf("MAIN: Koniec.\n");
}
