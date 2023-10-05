#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

int main() {
    //zmienne oraz losowanie znaku do wyslania
    int gnaizdodp;
    int dlugosc;
    struct sockaddr_un adres;
    int rezultat;

    srand(getpid());
    char znak;
    znak = rand() % 26 + 97;

    //tworzenie secketow
    gnaizdodp = socket(AF_UNIX, SOCK_STREAM, 0);

    adres.sun_family = AF_UNIX;
    strcpy(adres.sun_path, "gniazdo_serwera");
    dlugosc = sizeof(adres);

    rezultat = connect(gnaizdodp, (struct sockaddr*)&adres, dlugosc);
    if (rezultat == -1) {
        perror("Blad laczenia socketa\n");
        return 1;
    }

    if (write(gnaizdodp, &znak, 1) == -1) {
        perror("Klient- blad pisania");
    }
    else {
        printf("Klient- wyslal znak %c\n", znak);
    }

    if (read(gnaizdodp, &znak, 1) == -1) {
        perror("Klient- niepoprawnie odczytany znak");
    }
    else {
        printf("Klient- odbral znak - %c\n", znak);
    }

    if (close(gnaizdodp) == -1) {
        perror("Klient- blad zamykania socketa");
    }
    printf("Klient- klient konczy dzialanie\n");
    return 0;
}
