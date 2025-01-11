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
    init_pair(1, COLOR_GREEN, COLOR_BLACK);   // Zelený text
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Žltý text
    init_pair(3, COLOR_BLUE, COLOR_BLACK);    // Modrý text
    init_pair(4, COLOR_CYAN, COLOR_BLACK);    // Svetlomodrý text
    init_pair(5, COLOR_RED, COLOR_BLACK);     // Červený text
}

void start_server_if_needed(int *vyska, int *sirka, int *rezim, int *typ) {
    int status = system("ss -tuln | grep -q :8082");
    if (status != 0) { // Ak server nebeží
        initscr();
        noecho();
        cbreak();
        init_colors();
        
        attron(COLOR_PAIR(2)); // Žltý text
        mvprintw(0, 0, "Server is not on. Turning on the server...");
        attroff(COLOR_PAIR(2));
        refresh();
        sleep(1); // Čas na zobrazenie správy

        // Získaj vstupy od užívateľa
        attron(COLOR_PAIR(4)); // Svetlomodrý text
        mvprintw(2, 0, "---------- INITIAL    MENU ---------");
        mvprintw(3, 0, "---------- CONFIG THE GAME ---------");
        attroff(COLOR_PAIR(4));

        attron(COLOR_PAIR(1)); // Zelený text
        mvprintw(5, 0, "Enter the width of the playing field ");
        attroff(COLOR_PAIR(1));
        echo();
        scanw("%d", sirka);

        attron(COLOR_PAIR(1)); // Zelený text
        mvprintw(6, 0, "Enter the height of the playing field ");
        attroff(COLOR_PAIR(1));
        scanw("%d", vyska);

        attron(COLOR_PAIR(3)); // Modrý text
        mvprintw(7, 0, "Enter game mode: (1 - STANDARD | 2 - TIMED): ");
        attroff(COLOR_PAIR(3));
        scanw("%d", rezim);

        attron(COLOR_PAIR(3)); // Modrý text
        mvprintw(8, 0, "Enter the type of game world: (1 - WITHOUT OBSTACLES | 2 - WITH OBSTACLES): ");
        attroff(COLOR_PAIR(3));
        scanw("%d", typ);
        noecho();

        refresh();

        pid_t pid = fork();
        if (pid == 0) {
            // Dieťa – spustí server
            execl("./server", "./server", NULL);
            perror("Spustenie servera zlyhalo");
            endwin();
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("Fork zlyhal");
            endwin();
            exit(EXIT_FAILURE);
        }
        sleep(1); // Počkaj sekundu, kým sa server spustí

        endwin(); // Ukončenie ncurses
    }
}

int main(int argc, char const* argv[]) {
    int status, client_fd;
    struct sockaddr_in serv_addr;
    int key = 0;
    bool game_over = false;
    int vyska = 0, sirka = 0, rezim = 0, typ = 0, score = 0, elapsed_time = 0;
    char **mapa = NULL; // Dynamická alokácia pre mriežku

    // Skontroluj a spusti server, ak nie je spustený
    start_server_if_needed(&vyska, &sirka, &rezim, &typ);

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
    if(rezim > 0 && typ > 0) {
        send(client_fd, &rezim, sizeof(int), 0);
        send(client_fd, &typ, sizeof(int), 0);

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

