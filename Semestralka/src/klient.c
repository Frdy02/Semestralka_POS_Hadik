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
#include <sys/wait.h>

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

void start_server_if_needed(int* vyska, int* sirka) {
    int status = system("ss -tuln | grep -q :8082");
    if (status != 0) { // Ak server nebeží
        printf("Server nie je spustený. Spúšťam server...\n");

        // Získaj vstupy od užívateľa
        printf("Zadajte šírku hracej plochy: ");
        scanf("%d", vyska);
        printf("Zadajte výšku hracej plochy: ");
        scanf("%d", sirka);

        pid_t pid = fork();
        if (pid == 0) {
            // Dieťa – spustí server
            execl("./server", "./server", NULL);
            perror("Spustenie servera zlyhalo");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("Fork zlyhal");
            exit(EXIT_FAILURE);
        }
        sleep(1); // Počkaj sekundu, kým sa server spustí
    }
}

int main(int argc, char const* argv[]) {
    int status, client_fd;
    struct sockaddr_in serv_addr;
    int key = 0;
    bool game_over = false;
    int vyska = 0, sirka = 0, skore = 0, elapsed_time = 0;
    char **mapa = NULL; // Dynamická alokácia pre mriežku

    // Skontroluj a spusti server, ak nie je spustený
    start_server_if_needed(&vyska, &sirka);

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

    // Ak server nebežal, pošli dĺžku a šírku na server
    if (vyska > 0 && sirka > 0) {
        send(client_fd, &vyska, sizeof(int), 0);
        send(client_fd, &sirka, sizeof(int), 0);
    }

    // Dynamická alokácia mriežky
    mapa = malloc(sirka * sizeof(char*));
    if (!mapa) {
        perror("Nepodarilo sa alokovať mriežku");
        close(client_fd);
        return -1;
    }

    for (int i = 0; i < sirka; i++) {
        mapa[i] = malloc(vyska * sizeof(char));
        if (!mapa[i]) {
            perror("Nepodarilo sa alokovať riadok mriežky");
            for (int j = 0; j < i; j++) {
                free(mapa[j]);
            }
            free(mapa);
            close(client_fd);
            return -1;
        }
    }

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
        for (int i = 0; i < sirka; i++) {
            read(client_fd, mapa[i], vyska * sizeof(char));
        }
        read(client_fd, &skore, sizeof(int));
        read(client_fd, &elapsed_time, sizeof(elapsed_time));

        clear();
        mvprintw(0, 0, "Skóre: %d | Čas: %d s", skore, elapsed_time);

        // Vykreslenie mriežky
        for (int i = 0; i < sirka; i++) {
            for (int j = 0; j < vyska; j++) {
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

    // Uvoľnenie pamäte
    for (int i = 0; i < sirka; i++) {
        free(mapa[i]);
    }
    free(mapa);

    // Ukončenie hry
    endwin();
    close(client_fd);
    return 0;
}

