#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <ncurses.h>
#include <fcntl.h> // Pre non-blocking režim
#include "world.h"
#include <errno.h>
#include <time.h>
 
#define MAX_CLIENTS 10
#define PORT 26001
typedef struct {
    pthread_t thready[MAX_CLIENTS];
    int client_fd[MAX_CLIENTS];
    int client_count;
    bool is_world_initialized;
    pthread_mutex_t lock;
    pthread_cond_t all_ready_cond;
    int buffer[MAX_CLIENTS];
    int ready_count;
    int sirka;
    int vyska;
    int rezim;
    int typ;
    World world;
    int multi;
    int timegame;
    int elapsed_time;
} ServerState;
 
// Funkcia na nastavenie non-blocking režimu
int set_nonblocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL failed");
        return -1;
    }
 
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL failed");
        return -1;
    }
 
    return 0;
}
 
void* computation_loop(void* arg) {
    ServerState* state = (ServerState*)arg;
    int local_buffer[MAX_CLIENTS];
 
    //pthread_mutex_lock(&state->lock);
    if (!state->is_world_initialized) {
        
 
        read(state->client_fd[0], &state->sirka, sizeof(int));
        read(state->client_fd[0], &state->vyska, sizeof(int));
        read(state->client_fd[0], &state->rezim, sizeof(int));
        if (state->rezim == 2) {
            read(state->client_fd[0], &state->timegame, sizeof(int));
    }
        read(state->client_fd[0], &state->typ, sizeof(int));
        read(state->client_fd[0], &state->multi, sizeof(int));
 
        world_init(&state->world, state->sirka, state->vyska, state->rezim, state->typ);
       
        state->is_world_initialized = true;
 
    }
    //pthread_mutex_unlock(&state->lock);
    time_t start_time = time(NULL);
    while (!state->world.game_over) {
        time_t current_time = time(NULL);
        

        usleep(200000);
        state->elapsed_time = (int)(current_time - start_time);
        pthread_mutex_lock(&state->lock);
        
        while (state->ready_count < state->client_count) {
            pthread_cond_wait(&state->all_ready_cond, &state->lock);
        }
 
        for (int i = 0; i < state->client_count; i++) {
            local_buffer[i] = state->buffer[i];
            state->buffer[i] = 0;
        }
        state->ready_count = 0;
        pthread_mutex_unlock(&state->lock);
 
        world_update(&state->world, local_buffer);
 
        pthread_mutex_lock(&state->lock);
        for (int j = 0; j < state->client_count; j++) {
            int client_fd = state->client_fd[j];
            if (state->rezim == 2 && state->elapsed_time >= state->timegame) {
                state->world.game_over = true;
            }
             // Preskočiť odpojených klientov
            send(client_fd, &state->world.snakes[j].dead, sizeof(bool), 0);
            for (int i = 0; i < state->world.height; i++) {
                send(client_fd, state->world.grid[i], state->world.width * sizeof(char), 0);
            }
            send(client_fd, &state->world.snakes[j].length, sizeof(int), 0);
            send(client_fd, &state->elapsed_time, sizeof(int), 0);
        }
        pthread_mutex_unlock(&state->lock);
    }
 
    return NULL;
}
 
void* client_handler(void* arg) {
    ServerState* state = (ServerState*)arg;
    
    int client_idx = state->client_count - 1;
    
    int client_fd = state->client_fd[client_idx];
    
    int received_value;
   
    pthread_mutex_lock(&state->lock);
    while (1) {
        sleep(5);
        
            if (client_idx <= state->client_count && state->is_world_initialized) {
                
                world_add_player(&state->world);
            break;
        }
    }
    pthread_mutex_unlock(&state->lock);
 
    send(client_fd, &state->sirka, sizeof(int), 0);
    send(client_fd, &state->vyska, sizeof(int), 0);
    send(client_fd, &state->rezim, sizeof(int), 0);
    if (state->rezim == 2) {
            send(client_fd, &state->timegame, sizeof(int), 0);
        }
    send(client_fd, &state->typ, sizeof(int), 0);
    send(client_fd, &state->multi, sizeof(int), 0);
    

    
    
    bool zijem = true;
    while (!state->world.game_over && state->is_world_initialized && zijem) {
        ssize_t bytes_read = read(client_fd, &received_value, sizeof(int));
        if (bytes_read <= 0) {
            printf("Client %d disconnected.\n", client_idx + 1);
            close(client_fd);
 
            pthread_mutex_lock(&state->lock);
            state->client_fd[client_idx] = -1;
            state->client_count--;
            state->ready_count--;
            state->world.snakes[client_idx].dead = true;
             
            state->buffer[client_idx] = received_value;
            state->ready_count++;
            if (state->ready_count == state->client_count) {
                pthread_cond_signal(&state->all_ready_cond);
            }
            pthread_mutex_unlock(&state->lock);
 
            zijem = false;
        }
 
        if (received_value == -1) {
            printf("Client %d signaled disconnection.\n", client_idx + 1);
            close(client_fd);

            pthread_mutex_lock(&state->lock);
            state->client_fd[client_idx] = -1;
            state->client_count--;
            state->ready_count--;
            state->world.snakes[client_idx].dead = true;
           
            
            state->buffer[client_idx] = received_value;
            state->ready_count++;
            if (state->ready_count == state->client_count) {
                pthread_cond_signal(&state->all_ready_cond);
            }
            pthread_mutex_unlock(&state->lock);
 
            zijem = false;
        }
 
 
        pthread_mutex_lock(&state->lock);
        state->buffer[client_idx] = received_value;
        state->ready_count++;
        if (state->ready_count == state->client_count) {
            pthread_cond_signal(&state->all_ready_cond);
        }
        pthread_mutex_unlock(&state->lock);
    }
    //pthread_detach(state->thready[client_idx]);
    state->thready[client_idx] = 0;
    return NULL;
}
 
int main() {
    
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    ServerState state = {
        .client_count = 0,
        .is_world_initialized = false,
        .ready_count = 0,
        .sirka = 0,
        .vyska = 0,
        .rezim = 0,
        .typ = 0,
        .multi = 0,
        .timegame = 0
    };
    pthread_mutex_init(&state.lock, NULL);
    pthread_cond_init(&state.all_ready_cond, NULL);
 
   
 
    for (int i = 0; i < MAX_CLIENTS; i++) {
        state.client_fd[i] = -1;
    }
 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
 
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
 
    // Nastavenie non-blocking režimu
    if (set_nonblocking(server_fd) == -1) {
        close(server_fd);
        exit(EXIT_FAILURE);
    }
 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
 
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
 
    int prichod = 0;
    pthread_t computation_thread;
   
 
    do {
        new_socket = accept(server_fd, (struct sockaddr*)&address, &addr_len);
        if (state.world.game_over) {
            break;
        }
        if (new_socket < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(200000); 
                continue;
            } else {
                perror("Accept failed");
               
                break;
            }
        }
        if (new_socket > 0) {
            pthread_mutex_lock(&state.lock);
            if (state.client_count < MAX_CLIENTS) {
                state.client_fd[state.client_count++] = new_socket;
                prichod++;

                if (prichod == 1) {
                    pthread_create(&computation_thread, NULL, computation_loop, (void*)&state);
                    sleep(5);
                    prichod++;
                }
                pthread_t thread_id = state.thready[state.client_count - 1];
                pthread_create(&thread_id, NULL, client_handler, (void*)&state);
                pthread_detach(thread_id);
 
 
            } else {
                printf("Max clients reached.\n");
                close(new_socket);
            }
            
            pthread_mutex_unlock(&state.lock);
            if(state.multi == 0 && state.is_world_initialized) {
                    break;
            }
        }
       
    } while (state.client_count > 0 || prichod == 0 || !state.world.game_over);
    


    close(server_fd);
    pthread_join(computation_thread, NULL);
    pthread_mutex_destroy(&state.lock);
    pthread_cond_destroy(&state.all_ready_cond);
    world_free(&state.world);
    exit(0);
}
