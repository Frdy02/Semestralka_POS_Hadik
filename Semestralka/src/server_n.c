#include "world.h"
#include "input.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#define PORT 8082
#define MAX_CLIENTS 10

typedef struct {
    int client_socket;
    World *world;
    pthread_mutex_t *world_lock;
} ClientHandlerArgs;

void* client_handler(void* args) {
    ClientHandlerArgs *handler_args = (ClientHandlerArgs*)args;
    int client_socket = handler_args->client_socket;
    World *world = handler_args->world;
    pthread_mutex_t *world_lock = handler_args->world_lock;
    int client_input = 0;

    while (1) {
        // Čítanie vstupu od klienta
        ssize_t bytes_read = read(client_socket, &client_input, sizeof(int));
        if (bytes_read <= 0) {
            printf("Klient sa odpojil.\n");
            close(client_socket);
            free(handler_args);
            return NULL;
        }

        // Aktualizácia sveta
        pthread_mutex_lock(world_lock);
        world_update(world, client_input);
        pthread_mutex_unlock(world_lock);

        // Odoslanie stavu hry klientovi
        pthread_mutex_lock(world_lock);
        bool game_over = world->game_over;
        send(client_socket, &game_over, sizeof(bool), 0);
        for (int i = 0; i < world->height; i++) {
            send(client_socket, world->grid[i], world->width * sizeof(char), 0);
        }
        int snake_length = world->snake.length;
        send(client_socket, &snake_length, sizeof(int), 0);
        pthread_mutex_unlock(world_lock);
    }
    return NULL;
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    pthread_t threads[MAX_CLIENTS];
    int client_count = 0;

    // Inicializácia sveta
    World world;
    pthread_mutex_t world_lock;
    pthread_mutex_init(&world_lock, NULL);

    // Vytvorenie soketu
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server beží na porte %d\n", PORT);

    while (1) {
        if (client_count >= MAX_CLIENTS) {
            printf("Maximálny počet klientov dosiahnutý.\n");
            break;
        }

        int new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Pripojený nový klient.\n");

        if (client_count == 0) {
            // Od prvého klienta prečítame rozmery sveta
            read(new_socket, &world.height, sizeof(int));
            read(new_socket, &world.width, sizeof(int));
            world_init(&world, world.width, world.height);
        }

        // Vytvorenie vlákna pre klienta
        ClientHandlerArgs *handler_args = malloc(sizeof(ClientHandlerArgs));
        handler_args->client_socket = new_socket;
        handler_args->world = &world;
        handler_args->world_lock = &world_lock;

        pthread_create(&threads[client_count], NULL, client_handler, handler_args);
        client_count++;
    }

    close(server_fd);
    pthread_mutex_destroy(&world_lock);
    return 0;
}