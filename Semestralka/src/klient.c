#define EMPTY ' ' // Prázdno
#define WALL '#'  // Stena
#define SNAKE 'O' // Had
#define FRUIT '*' // Ovocie

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

#define PORT 8082

void init_colors() {
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);   // Farba pre ovocie
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Farba pre hada
    init_pair(3, COLOR_BLUE, COLOR_BLACK);    // Farba pre steny
}

int main(int argc, char const* argv[]) {
    int status, client_fd;
    struct sockaddr_in serv_addr;
    int key = 0;
    char mapa[20][20] = { 0 }; // Fixná veľkosť pre ukážku
    bool game_over = false;
    int sirka = 0, dlzka = 0, skore = 0, elapsed_time = 0;

    // Vytvorenie socketu
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

    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        perror("Connection failed");
        return -1;
    }

    // Čítanie rozmerov mriežky zo servera
    read(client_fd, &sirka, sizeof(int));
    read(client_fd, &dlzka, sizeof(int));

    // Inicializácia ncurses
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);

    // Inicializácia farieb
    init_colors();

    // Herná slučka
    while (!game_over) {
        usleep(125000);

        // Odoslanie vstupu hráča na server
        input(&key);
        send(client_fd, &key, sizeof(int), 0);

        // Čítanie stavu hry a mriežky zo servera
        read(client_fd, &game_over, sizeof(bool));
        read(client_fd, &mapa, sizeof(mapa));
        read(client_fd, &skore, sizeof(int));
        read(client_fd, &elapsed_time, sizeof(elapsed_time));


        clear();
        mvprintw(0, 0, "Score: %d | Time: %d s", skore, elapsed_time);

        // Vykreslenie mriežky
        
        for (int i = 0; i < dlzka; i++) {
            for (int j = 0; j < sirka; j++) {
                char c = mapa[i][j];
                switch (c) {
                    case FRUIT:
                        attron(COLOR_PAIR(1));
                        mvaddch(i + 1, j, c);
                        attroff(COLOR_PAIR(1));
                        break;
                    case SNAKE:
                        attron(COLOR_PAIR(2));
                        mvaddch(i + 1, j, c);
                        attroff(COLOR_PAIR(2));
                        break;
                    case WALL:
                        attron(COLOR_PAIR(3));
                        mvaddch(i + 1, j, c);
                        attroff(COLOR_PAIR(3));
                        break;
                    default:
                        mvaddch(i + 1, j, c);
                }
            }
        }
        refresh();
    }

    // Ukončenie hry
    endwin();
    close(client_fd);
    return 0;
}
