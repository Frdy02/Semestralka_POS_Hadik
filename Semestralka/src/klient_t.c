#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 8080

int main() {
    int client_fd;
    struct sockaddr_in serv_addr;

    // Vytvorenie soketu
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Nastavenie adresy servera
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    // Pripojenie na server
    if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    printf("Pripojený na server.\n");

    int value;
    while (1) {
        printf("Zadajte číslo: ");
        scanf("%d", &value);

        // Odoslanie čísla na server
        send(client_fd, &value, sizeof(int), 0);
    }

    close(client_fd);
    return 0;
}
