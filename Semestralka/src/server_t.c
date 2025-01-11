#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <ncurses.h>
#include "world.h"

#define PORT 9090
#define MAX_CLIENTS 20

typedef struct {
    int client_fd[MAX_CLIENTS];
    int client_count;
    bool is_world_initialized;
    pthread_mutex_t lock;
    int buffer[MAX_CLIENTS];
    int top;
    int sirka;
    int vyska;
    int rezim;
    int typ;
    World world;
} ServerState;

void init_colors() {
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);
    init_pair(5, COLOR_RED, COLOR_BLACK);
}

void* computation_loop(void* arg) {
    ServerState* state = (ServerState*)arg;
    int local_buffer[MAX_CLIENTS];

    pthread_mutex_lock(&state->lock);
    if (!state->is_world_initialized) {
        printf("Initializing game world...\n");

        // NCurses initialization
        initscr();
        noecho();
        cbreak();
        keypad(stdscr, TRUE);
        curs_set(0);
        nodelay(stdscr, TRUE);
        init_colors();

        // Configure game world
        clear();
        mvprintw(0, 0, "Enter the width of the playing field: ");
        scanw("%d", &state->sirka);
        mvprintw(1, 0, "Enter the height of the playing field: ");
        scanw("%d", &state->vyska);
        mvprintw(2, 0, "Enter game mode (1 - Standard, 2 - Timed): ");
        scanw("%d", &state->rezim);
        mvprintw(3, 0, "Enter the type of world (1 - Without Obstacles, 2 - With Obstacles): ");
        scanw("%d", &state->typ);

        refresh();

        world_init(&state->world, state->sirka, state->vyska, state->rezim, state->typ);
        state->is_world_initialized = true;

        clear();
        refresh();
    }
    pthread_mutex_unlock(&state->lock);

    while (!state->world.game_over) {
        usleep(200000); // 200 ms

        pthread_mutex_lock(&state->lock);
        for (int i = 0; i < state->top; i++) {
            local_buffer[i] = state->buffer[i];
        }

        state->top = 0;
        
        
        // Vynulovanie state->buffer
        pthread_mutex_lock(&state->lock);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            state->buffer[i] = 0; // Nastavenie na 0
        }
        state->top = 0; // Resetovanie vrcholu bufferu
        pthread_mutex_unlock(&state->lock);
        
                
        
        
        pthread_mutex_unlock(&state->lock);

        world_update(&state->world, local_buffer);

        pthread_mutex_lock(&state->lock);
        clear();
        mvprintw(0, 0, "Game is running...");
        mvprintw(1, 0, "Players connected: %d", state->client_count);
        refresh();
        pthread_mutex_unlock(&state->lock);

        // Broadcast updated world state to all clients
        for (int j = 0; j < state->client_count; j++) {
            int client_fd = state->client_fd[j];
            send(client_fd, &state->world.game_over, sizeof(bool), 0);
            for (int i = 0; i < state->world.height; i++) {
                send(client_fd, state->world.grid[i], state->world.width * sizeof(char), 0);
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            local_buffer[i] = 0; // Nastavenie na 0
        }
    }

    return NULL;
}


void* client_handler(void* arg) {
    ServerState* state = (ServerState*)arg;
    int client_idx = state->client_count - 1;
    int client_fd = state->client_fd[client_idx];
    int received_value;

    // Pridať nového hráča do sveta
    world_add_player(&state->world);

    while (!state->world.game_over) {
        ssize_t bytes_read = read(client_fd, &received_value, sizeof(int));
        if (bytes_read <= 0) {
            printf("Client %d disconnected.\n", client_idx + 1);
            close(client_fd);

            // Znížiť počet klientov a uvoľniť miesto pre nových
            pthread_mutex_lock(&state->lock);
            state->client_fd[client_idx] = -1;
            state->client_count--;
            pthread_mutex_unlock(&state->lock);

            pthread_exit(NULL);
        }

        // Uložiť prijatý vstup do bufferu
        pthread_mutex_lock(&state->lock);
        if (state->top < MAX_CLIENTS) { // Zabezpečiť, aby nedošlo k pretečeniu
            state->buffer[state->top++] = received_value;
        }
        pthread_mutex_unlock(&state->lock);
    }

    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    ServerState state = {
        .client_count = 0,
        .is_world_initialized = false,
        .top = 0,
        .sirka = 20,
        .vyska = 20,
        .rezim = 1,
        .typ = 1
    };
    pthread_mutex_init(&state.lock, NULL);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        state.client_fd[i] = -1;
    }

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server running on port %d\n", PORT);

    pthread_t computation_thread;
    pthread_create(&computation_thread, NULL, computation_loop, (void*)&state);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addr_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_mutex_lock(&state.lock);
        if (state.client_count < MAX_CLIENTS) {
            state.client_fd[state.client_count++] = new_socket;
            printf("Client %d connected.\n", state.client_count);

            send(new_socket, &state.sirka, sizeof(int), 0);
            send(new_socket, &state.vyska, sizeof(int), 0);
            send(new_socket, &state.rezim, sizeof(int), 0);
            send(new_socket, &state.typ, sizeof(int), 0);

            pthread_t thread_id;
            pthread_create(&thread_id, NULL, client_handler, (void*)&state);
        } else {
            printf("Max clients reached.\n");
            close(new_socket);
        }
        pthread_mutex_unlock(&state.lock);
    }

    close(server_fd);
    pthread_mutex_destroy(&state.lock);
    endwin();
    world_free(&state.world);

    return 0;
}
