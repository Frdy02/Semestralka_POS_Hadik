#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include "gamestate.h"

GameState game_state;
pthread_mutex_t game_lock;
int current_direction = 2;
int client_connected = 0;
int game_running = 0;

void reset_game() {
    pthread_mutex_lock(&game_lock);
    gamestate_init(&game_state, "hernysvet.txt", "ovocie.txt");
    current_direction = 2;
    game_running = 1;
    pthread_mutex_unlock(&game_lock);
    printf("Hra bola reštartovaná\n");
}

void *client_input_thread(void *arg) {
    int client_sock = *(int *)arg;
    char input;

    while (1) {
        if (read(client_sock, &input, sizeof(input)) <= 0) {
            printf("Klient sa odpojil\n");
            client_connected = 0;
            break;
        }

        if (input == '1') { // Reštart
            printf("Klient požiadal o reštart hry\n");
            reset_game();
        } else if (input == '2') { // Odísť
            printf("Klient požiadal o ukončenie\n");
            client_connected = 0;
            game_running = 0;
            break;
        } else {
            pthread_mutex_lock(&game_lock);
            if (input == 'w') current_direction = 0;
            if (input == 'a') current_direction = 1;
            if (input == 's') current_direction = 2;
            if (input == 'd') current_direction = 3;
            pthread_mutex_unlock(&game_lock);
        }
    }

    close(client_sock);
    return NULL;
}

void *game_update_thread(void *arg) {
    int client_sock = *(int *)arg;

    while (client_connected) {
        pthread_mutex_lock(&game_lock);
        if (game_running && !gamestate_is_game_over(&game_state)) {
            gamestate_update(&game_state, current_direction);
        }
        pthread_mutex_unlock(&game_lock);

        if (client_connected) {
            if (write(client_sock, &game_state, sizeof(game_state)) <= 0) {
                printf("Chyba pri posielaní dát klientovi\n");
                client_connected = 0;
                break;
            }
        }

        usleep(100000);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Použitie: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    int sockfd, client_sock;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Chyba pri vytváraní socketu");
        return 1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Chyba pri bind() na socket");
        close(sockfd);
        return 1;
    }

    if (listen(sockfd, 5) < 0) {
        perror("Chyba pri listen()");
        close(sockfd);
        return 1;
    }

    printf("Server počúva na porte %d...\n", port);

    gamestate_init(&game_state, "hernysvet.txt", "ovocie.txt");
    pthread_mutex_init(&game_lock, NULL);
    game_running = 1;

    while (1) {
        client_sock = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);
        if (client_sock < 0) {
            perror("Chyba pri accept()");
            continue;
        }

        printf("Klient pripojený.\n");
        client_connected = 1;

        pthread_t input_tid, update_tid;
        pthread_create(&input_tid, NULL, client_input_thread, &client_sock);
        pthread_create(&update_tid, NULL, game_update_thread, &client_sock);

        pthread_join(input_tid, NULL);
        pthread_join(update_tid, NULL);
    }

    pthread_mutex_destroy(&game_lock);
    close(sockfd);
    return 0;
}


