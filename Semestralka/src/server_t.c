#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 2

typedef struct {
    int client_fd[MAX_CLIENTS];
    int client_count;
    int x_client_1;
    int x_client_2;
    pthread_mutex_t lock;
} ServerState;

void* computation_loop(void* arg) {
    ServerState* state = (ServerState*)arg;

    while (1) {
        usleep(200000); // 200 ms

        pthread_mutex_lock(&state->lock);
        int x1 = state->x_client_1;
        int x2 = state->x_client_2;
        printf("Súčet hodnôt: %d + %d = %d\n", x1, x2, x1 + x2);
        pthread_mutex_unlock(&state->lock);
    }

    return NULL;
}

void* client_handler(void* arg) {
    ServerState* state = (ServerState*)arg;
    int client_idx = state->client_count - 1;
    int client_fd = state->client_fd[client_idx];
    int received_value = 0;

    while (1) {
        // Čakaj na správu od klienta
        ssize_t bytes_read = read(client_fd, &received_value, sizeof(int));
        if (bytes_read <= 0) {
            printf("Klient %d sa odpojil.\n", client_idx + 1);
            close(client_fd);
            state->client_fd[client_idx] = -1; // Označíme klienta ako odpojeného
            pthread_exit(NULL);
        }

        pthread_mutex_lock(&state->lock);
        if (client_idx == 0) {
            state->x_client_1 = received_value;
        } else if (client_idx == 1) {
            state->x_client_2 = received_value;
        }
        pthread_mutex_unlock(&state->lock);

        printf("Prijaté od klienta %d: %d\n", client_idx + 1, received_value);
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    ServerState state = { .client_count = 0, .x_client_1 = 0, .x_client_2 = 0 };
    pthread_mutex_init(&state.lock, NULL);

    // Nastavenie klientov ako neaktívnych
    for (int i = 0; i < MAX_CLIENTS; i++) {
        state.client_fd[i] = -1;
    }

    // Vytvorenie soketu
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Nastavenie adresy
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Pripojenie soketu na port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server beží na porte %d\n", PORT);

    // Spustenie vlákna na výpočet
    pthread_t computation_thread;
    pthread_create(&computation_thread, NULL, computation_loop, (void*)&state);

    // Prijímanie klientov
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addr_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_mutex_lock(&state.lock);
        if (state.client_count < MAX_CLIENTS) {
            state.client_fd[state.client_count] = new_socket;
            state.client_count++;
            printf("Pripojený klient %d\n", state.client_count);
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, client_handler, &state);
        } else {
            printf("Maximálny počet klientov dosiahnutý.\n");
            close(new_socket);
        }
        pthread_mutex_unlock(&state.lock);
    }

    close(server_fd);
    pthread_mutex_destroy(&state.lock);
    return 0;
}
