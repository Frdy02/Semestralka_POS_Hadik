#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "gamestate.h"
#include "input.h"

GameState game_state;
int running = 1;
int sockfd;
pthread_mutex_t lock;

int check_server_running(const char *ip, int port) {
    int temp_sockfd;
    struct sockaddr_in serv_addr;

    temp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (temp_sockfd < 0) {
        perror("Chyba pri vytváraní socketu");
        return 0;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        close(temp_sockfd);
        return 0;
    }

    if (connect(temp_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(temp_sockfd);
        return 0;
    }

    close(temp_sockfd);
    return 1;
}

void start_server(const char *port) {
    pid_t pid = fork();

    if (pid == 0) {
        execlp("./server", "./server", port, (char *)NULL);
        perror("Chyba pri spúšťaní servera");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Chyba pri fork()");
        exit(EXIT_FAILURE);
    }

    sleep(1);
}

void *receive_thread(void *arg) {
    while (running) {
        GameState temp_state;

        if (read(sockfd, &temp_state, sizeof(temp_state)) <= 0) {
            printf("Spojenie so serverom bolo ukončené\n");
            running = 0;
            break;
        }

        pthread_mutex_lock(&lock);
        memcpy(&game_state, &temp_state, sizeof(GameState));
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

void *input_thread(void *arg) {
    char input_p;
    while (running) {
        if (kbhit()) {
            input_p = getchar();

            if (write(sockfd, &input_p, sizeof(input_p)) <= 0) {
                perror("Chyba pri posielaní vstupu serveru");
                running = 0;
                break;
            }
        }
        usleep(50000);
    }
    return NULL;
}

void show_menu() {
    disable_raw_mode();
    printf("1 - Reštartovať hru\n");
    printf("2 - Odísť\n");
    char choice;
    scanf(" %c", &choice);

    if (write(sockfd, &choice, sizeof(choice)) <= 0) {
        perror("Chyba pri posielaní výberu serveru");
        running = 0;
    }

    if (choice == '2') {
        running = 0;
    }

    enable_raw_mode();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Použitie: %s <port>\n", argv[0]);
        return 1;
    }

    const char *port = argv[1];
    int port_num = atoi(port);

    if (!check_server_running("127.0.0.1", port_num)) {
        printf("Server nebeží. Automaticky ho spúšťam...\n");
        start_server(port);
    } else {
        printf("Server už beží na porte %s\n", port);
    }

    struct sockaddr_in serv_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Chyba pri vytváraní socketu");
        return 1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_num);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Chyba pri nastavovaní IP adresy");
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Chyba pri pripojení na server");
        return 1;
    }

    printf("Pripojené na server\n");

    enable_raw_mode();
    pthread_mutex_init(&lock, NULL);

    pthread_t recv_tid, input_tid;
    pthread_create(&recv_tid, NULL, receive_thread, NULL);
    pthread_create(&input_tid, NULL, input_thread, NULL);

    while (running) {
        pthread_mutex_lock(&lock);
        gamestate_draw(&game_state);
        pthread_mutex_unlock(&lock);

        if (gamestate_is_game_over(&game_state)) {
            printf("Hra skončila! Skóre: %d\n", game_state.score);
            show_menu();
        }

        usleep(100000);
    }

    pthread_join(recv_tid, NULL);
    pthread_join(input_tid, NULL);
    pthread_mutex_destroy(&lock);
    disable_raw_mode();
    close(sockfd);
    return 0;
}


