#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>
#include "input.h"

#define EMPTY ' ' // Prázdno
#define WALL '#'  // Stena
#define SNAKE 'O' // Had
#define FRUIT '*' // Ovocie

#define PORT 9090


void init_colors() {
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);   // Zelený text
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Žltý text
    init_pair(3, COLOR_BLUE, COLOR_BLACK);    // Modrý text
    init_pair(4, COLOR_CYAN, COLOR_BLACK);    // Svetlomodrý text
    init_pair(5, COLOR_RED, COLOR_BLACK);     // Červený text
}

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
    int client_fd, sirka, vyska, rezim, typ, key = 0, score = 0, elapsed_time = 0;
    bool game_over = false;
    struct sockaddr_in serv_addr;
    char **mapa;

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

    // Prijímanie parametrov hry
    read(client_fd, &sirka, sizeof(int));
    read(client_fd, &vyska, sizeof(int));
    read(client_fd, &rezim, sizeof(int));
    read(client_fd, &typ, sizeof(int));

    // Alokácia pamäte pre mriežku
    mapa = malloc(vyska * sizeof(char*));
    if (!mapa) {
        perror("Nepodarilo sa alokovať mriežku");
        close(client_fd);
        return -1;
    }

    for (int i = 0; i < vyska; i++) {
        mapa[i] = malloc(sirka * sizeof(char));
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
        for (int i = 0; i < vyska; i++) {
            read(client_fd, mapa[i], sirka * sizeof(char));
        }
        read(client_fd, &score, sizeof(int));
        read(client_fd, &elapsed_time, sizeof(elapsed_time));

        // Vyčistenie obrazovky
        clear();

        // Vypísanie skóre a času
        attron(A_BOLD | COLOR_PAIR(2)); // Zvýraznenie
        mvprintw(0, 0, "Score: %d | Time elapsed: %d s", score, elapsed_time);
        mvprintw(1, 0, "Game mode: %s | World type: %s",
                 (rezim == 1 ? "Standard" : "Timed"),
                 (typ == 1 ? "Without obstacles" : "With obstacles"));
        attroff(A_BOLD | COLOR_PAIR(2));

        // Vykreslenie mriežky
        for (int i = 0; i < vyska; i++) {
            for (int j = 0; j < sirka; j++) {
                char c = mapa[i][j];
                switch (c) {
                    case FRUIT:
                        attron(COLOR_PAIR(1));
                        mvaddch(i + 2, j, c);
                        attroff(COLOR_PAIR(1));
                        break;
                    case SNAKE:
                        attron(COLOR_PAIR(2));
                        mvaddch(i + 2, j, c);
                        attroff(COLOR_PAIR(2));
                        break;
                    case WALL:
                        attron(COLOR_PAIR(3));
                        mvaddch(i + 2, j, c);
                        attroff(COLOR_PAIR(3));
                        break;
                    default:
                        mvaddch(i + 2, j, c);
                }
            }
        }

        // Aktualizácia obrazovky
        refresh();
    }

    // Cleanup
    for (int i = 0; i < vyska; i++) {
        free(mapa[i]);
    }
    free(mapa);
    close(client_fd);
    endwin(); // Ukončenie ncurses
    printf("Hra skončila. Ďakujeme za hranie!\n");

    return 0;
}
