#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>

#define PORT 9090

void start_server_if_needed() {
    // Skontroluj, či server beží na porte 9090
    int status = system("ss -tuln | grep -q :9090");
    if (status != 0) { // Ak server nebeží
        printf("Server nie je spustený. Spúšťam server...\n");
        pid_t pid = fork();
        if (pid == 0) {
            // Dieťa – spustí server
            execl("./server_t", "./server_t", NULL);
            perror("Spustenie servera zlyhalo");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("Fork zlyhal");
            exit(EXIT_FAILURE);
        }
        // Počkajte, kým sa server spustí
        sleep(1);
    }
}

int main() {
    int client_fd;
    struct sockaddr_in serv_addr;

    // Spustenie servera, ak je to potrebné
    start_server_if_needed();

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
