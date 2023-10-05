#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>

void my_exit() {
    unlink("gniazdo_serwera");
    printf("Serwer: koniec pracy\n");
    exit(0);
}

int main() {
    //deklaracje zmiennych
    int gniazdo_serweradp;
    int gniazdo_klientadp;
    int dlugosc_serwer;
    unsigned dlugosc_klient;
    struct sockaddr_un adres_serwera;
    struct sockaddr_un adres_klienta;

    //obsluga sygnalow
    signal(SIGINT, my_exit);
    signal(SIGTERM, my_exit);

    //konfiguracja serwera
    gniazdo_serweradp = socket(AF_UNIX, SOCK_STREAM, 0);

    adres_serwera.sun_family = AF_UNIX;
    strcpy(adres_serwera.sun_path, "gniazdo_serwera");
    dlugosc_serwer = sizeof(adres_serwera);
    if (bind(gniazdo_serweradp, (struct sockaddr*)&adres_serwera, dlugosc_serwer) == -1) {
        perror("S: blad bind\n");
        my_exit();
    }
    //czeka na polaczanie do zaakcpetowania
    if (listen(gniazdo_serweradp, 5) == -1) {
        perror("Serwer: blad listen\n");
        my_exit();
    }

    while (1) {
        char znak;
        printf("Serwer: czeka na polaczenie\n");
        gniazdo_klientadp = accept(gniazdo_serweradp, (struct sockaddr*)&adres_klienta, &dlugosc_klient);
        if (gniazdo_klientadp == -1) {
            perror("Serwer- blad accept\n");
        }
        if (read(gniazdo_klientadp, &znak, 1) == -1) {
            perror("Serwer- Blad czytania\n");
        }
        else {
            printf("Serwer- odczytano znak od klienta: %c\n", znak);
        }

        znak = toupper(znak);

        if (write(gniazdo_klientadp, &znak, 1) == -1) {
            perror("Serwer- blad pisania\n");
        }
        else {
            printf("Serwer- Wyslano znak do klienta: %c\n", znak);
        }

        if (close(gniazdo_klientadp) == -1) {
            perror("Serwer- Blad zamykania socketa klienta");
        }
    }
}

