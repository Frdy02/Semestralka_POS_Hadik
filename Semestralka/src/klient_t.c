#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>
#include "input.h"
#include <stdbool.h>
#include <ncurses.h>

#define EMPTY ' ' // Prázdno
#define WALL '#'  // Stena
#define SNAKE 'O' // Had
#define FRUIT '*' // Ovocie
#define OBSTACLE 'X' // Prekážka

#define PORT 26001
void init_colors() {
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);   // Zelený text
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Žltý text
    init_pair(3, COLOR_BLUE, COLOR_BLACK);    // Modrý text
    init_pair(4, COLOR_CYAN, COLOR_BLACK);    // Svetlomodrý text
    init_pair(5, COLOR_RED, COLOR_BLACK);     // Červený text
}

void start_server_if_needed(int *multi, int *vyska, int *sirka, int *rezim, int *timegame, int *typ) {
    // Skontroluj, či server beží na porte 26001
    int status = system("ss -tuln | grep -q :26001");
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

       
        attron(COLOR_PAIR(3)); // Svetlomodrý text
        mvprintw(4, 0, "Enable multi mode? (yes 1 | no 0): ");
        attroff(COLOR_PAIR(3));
        echo();
        scanw("%d", multi);
 
        attron(COLOR_PAIR(1)); // Zelený text
        mvprintw(5, 0, "Enter the width of the playing field: ");
        attroff(COLOR_PAIR(1));
        scanw("%d", sirka);
 
        attron(COLOR_PAIR(1)); // Zelený text
        mvprintw(6, 0, "Enter the height of the playing field: ");
        attroff(COLOR_PAIR(1));
        scanw("%d", vyska);
 
        attron(COLOR_PAIR(3)); // Modrý text
        mvprintw(7, 0, "Enter game mode: (1 - STANDARD | 2 - TIMED): ");
        attroff(COLOR_PAIR(3));
        scanw("%d", rezim);
 
        if(*rezim == 2) {
            attron(COLOR_PAIR(3)); // Modrý text
            mvprintw(8, 0, "Enter the time for TIMED gamemode: ");
            attroff(COLOR_PAIR(3));
            scanw("%d", timegame);
        }
 
        attron(COLOR_PAIR(3)); // Modrý text
        mvprintw(9, 0, "Enter the type of game world: (1 - WITHOUT OBSTACLES | 2 - WITH OBSTACLES): ");
        attroff(COLOR_PAIR(3));
        scanw("%d", typ);
        noecho();
 
        refresh();
 

        
    
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
 
        sleep(5);
 
        endwin();
 
        // Počkajte, kým sa server spustí
        sleep(1);
    }
}

int main() {
    int client_fd, sirka = 0,
    vyska = 0,
    rezim = 0, 
    typ = 0, 
    key = 0, 
    score = 0, 
    elapsed_time = 0,
    multi;
    bool game_over = false;
    struct sockaddr_in serv_addr;
    char **mapa;
    int timegame;
    

    // Spustenie servera, ak je to potrebné
    start_server_if_needed(&multi, &vyska, &sirka, &rezim, &timegame , &typ);

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

    int max_retries = 10;
    int retry_count = 0;
    while (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        if (retry_count++ >= max_retries) {
            perror("Connection failed after retries");
            exit(EXIT_FAILURE);
        }
        printf("Retrying connection...\n");
        sleep(1); // Počkajte pred ďalším pokusom
    }

    
    printf("Pripojený na server.\n");

    if (vyska > 0 && sirka > 0) {
        sleep(5);
        
        send(client_fd, &sirka, sizeof(int), 0);
        send(client_fd, &vyska, sizeof(int), 0);
        send(client_fd, &rezim, sizeof(int), 0);
        if (rezim == 2) {
            send(client_fd, &timegame, sizeof(int), 0);
        }
        send(client_fd, &typ, sizeof(int), 0);
        send(client_fd, &multi, sizeof(int), 0);
        
        

    }
    
    // Prijímanie parametrov hry
    read(client_fd, &sirka, sizeof(int));
    read(client_fd, &vyska, sizeof(int));
    read(client_fd, &rezim, sizeof(int));
    if (rezim == 2) {
            read(client_fd, &timegame, sizeof(int));
    }
    read(client_fd, &typ, sizeof(int));
    read(client_fd, &multi, sizeof(int));
    

    printf("%d  %d  %d  %d", sirka , vyska , rezim , typ);
   
    
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
    game_over = false;
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

        // Vyčistenie obrazovky
        clear();
        read(client_fd, &score, sizeof(int));
        read(client_fd, &elapsed_time, sizeof(int));
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
                    case OBSTACLE:
                        attron(COLOR_PAIR(5));
                        mvaddch(i + 2, j, c);
                        attroff(COLOR_PAIR(5));
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

    int disconnect_signal = -1; // Signál pre odpojenie klienta
    send(client_fd, &disconnect_signal, sizeof(int), 0);
    


    free(mapa);
    close(client_fd);
    echo();
    endwin(); // Ukončenie ncurses
    
    printf("Hra skončila. Ďakujeme za hranie!\n");

    return 0;
}
