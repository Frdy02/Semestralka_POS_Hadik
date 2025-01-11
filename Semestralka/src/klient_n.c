#include "input.h"
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <ncurses.h>
#include <stdlib.h>

#define PORT 8082
#define EMPTY ' ' // Prázdno
#define WALL '#'  // Stena
#define SNAKE 'O' // Had
#define FRUIT '*' // Ovocie

void init_colors() {
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);   // Farba pre ovocie
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Farba pre hada
    init_pair(3, COLOR_BLUE, COLOR_BLACK);    // Farba pre steny
}

void send_dimensions_if_needed(int client_fd, int* width, int* height) {
    int status = system("ss -tuln | grep -q :8082");
    if (status != 0) {
        printf("Server nie je spustený. Zadajte rozmery hracej plochy:\n");
        printf("Šírka: ");
        scanf("%d", width);
        printf("Výška: ");
        scanf("%d", height);
        send(client_fd, height, sizeof(int), 0);
        send(client_fd, width, sizeof(int), 0);
    }
}

int main() {
    int client_fd, key = 0;
    struct sockaddr_in serv_addr;
    bool game_over = false;
    int width = 0, height = 0, snake_length = 0;
    char **grid;

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    // Pošli rozmery, ak server nebeží
    send_dimensions_if_needed(client_fd, &width, &height);

    // Prijímanie rozmerov
    if (width == 0 && height == 0) {
        read(client_fd, &width, sizeof(int));
        read(client_fd, &height, sizeof(int));
    }

    // Alokácia gridu
    grid = malloc(height * sizeof(char*));
    for (int i = 0; i < height; i++) {
        grid[i] = malloc(width * sizeof(char));
    }

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);
    init_colors();

    while (!game_over) {
        usleep(125000);
        input(&key);
        send(client_fd, &key, sizeof(int), 0);

        read(client_fd, &game_over, sizeof(bool));
        for (int i = 0; i < height; i++) {
            read(client_fd, grid[i], width * sizeof(char));
        }
        read(client_fd, &snake_length, sizeof(int));

        clear();
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                char c = grid[i][j];
                switch (c) {
                    case FRUIT:
                        attron(COLOR_PAIR(1));
                        mvaddch(i, j, c);
                        attroff(COLOR_PAIR(1));
                        break;
                    case SNAKE:
                        attron(COLOR_PAIR(2));
                        mvaddch(i, j, c);
                        attroff(COLOR_PAIR(2));
                        break;
                    case WALL:
                        attron(COLOR_PAIR(3));
                        mvaddch(i, j, c);
                        attroff(COLOR_PAIR(3));
                        break;
                    default:
                        mvaddch(i, j, c);
                }
            }
        }
        refresh();
    }

    endwin();
    for (int i = 0; i < height; i++) {
        free(grid[i]);
    }
    free(grid);
    close(client_fd);
    return 0;
}