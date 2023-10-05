#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <time.h>
int main(int argc, char *argv[]) {
    
    long int num_s = strtol(argv[1], NULL, 10);
    
    FILE *fp;
    int id = getpid();
    char znak;
    char buf[20], fileName[20] = "temp/we";
    sprintf(buf, "%d", id);
    strcat(fileName, buf);
    
    fp = fopen(fileName, "w");

    if(fp == NULL) {
        perror("ERROR - tworzenie pliku\n");
        exit(EXIT_FAILURE);
    }
    
    for(int i = 0; i < num_s; ++i) {
        srand(time(NULL) - id*i*2);
        int x = 97 + (rand() % 25);
        znak = x;
        fputc(znak, fp);
        if(write(1, &znak, 1) == -1) {
            perror("ERROR - zapis do potoku\n");
            exit(EXIT_FAILURE);
        }
    }

    if(fclose(fp) == EOF) {
        perror("ERROR - zamykanie pliku\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

