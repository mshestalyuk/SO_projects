#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <time.h>
int main(int argc, char *argv[]) {
    int potok;
    FILE *fp;
    int id = getpid();
    char znak;
    char buf[20], fileName[20] = "temp/wy";
    sprintf(buf, "%d", id);
    strcat(fileName, buf);
    
    fp = fopen(fileName, "w");

    if(fp == NULL) {
        perror("ERROR - tworzenie pliku\n");
        exit(EXIT_FAILURE);
    }
    
    while(read(0, &znak, 1) > 0 ) {
        if(potok == -1) {
            perror("ERROR - czytanie potoku\n");
            exit(EXIT_FAILURE);
        }
        fputc(znak, fp);
    }

    if(fclose(fp) == EOF) {
        perror("ERROR - zamykanie pliku\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

