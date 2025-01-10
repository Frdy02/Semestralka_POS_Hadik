#define EMPTY ' ' // Prázdno
#define WALL '#'  // Stena
#define SNAKE 'O' // Had
#define FRUIT '*' // Ovocie
#define ZELENA "\033[1;32m"
#define ORANZOVA "\033[38;5;214m"
#define MODRA "\033[1;34m"
#define RESET_FARBA "\033[0m"
#include "input.h"
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080
static struct termios oldt, newt;

// Client side C program to demonstrate Socket
// programming


int main(int argc, char const* argv[])
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char* hello = "Hello from client";
    char buffer[1024] = { 0 };
    int key = 0;
    char mapa[20][20] = { 0 };
    bool game_over = false;
    int sirka = 0;
    int dlzka = 0;
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }


    if ((status
        = connect(client_fd, (struct sockaddr*)&serv_addr,
            sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    read(client_fd, &sirka, sizeof(int));
    read(client_fd, &dlzka, sizeof(int));
    printf("%d", sirka);
    printf("%d", dlzka);

    enable_raw_mode();
    while (!game_over) {
        usleep(200000);
        input(&key);
        send(client_fd, &key, sizeof(int), 0);
        read(client_fd, &game_over , sizeof(bool));
        read(client_fd, &mapa, sizeof(mapa));
        	
	    
        printf("\033[H\033[J");
        for (int i = 0; i < sirka; i++) {
            for (int j = 0; j < dlzka; j++) {
                char c = mapa[i][j];
            switch (c) {
            case FRUIT: printf(ZELENA "%c" RESET_FARBA, c); break;
            case SNAKE: printf(ORANZOVA "%c" RESET_FARBA, c); break;
            case WALL: printf(MODRA "%c" RESET_FARBA, c); break;
            default: printf("%c", c);
            
            }
            
            }
            printf("\n");
        }
        fflush(stdout);
        
    }

    disable_raw_mode();
    // closing the connected socket
    close(client_fd);
    return 0;
}


void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int kbhit(void) {
    struct timeval tv = { 0, 0 };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

char get_input(void) {
    return getchar();
}

void input(int* key) {
    if (kbhit()) {
        char user_input = get_input();

        // Spracovanie vstupu od používateľa
        switch (user_input) {
        case 'w':
            if (*key != 2)*key = 0;  // Hore
            break;
        case 'a':
            if (*key != 3)*key = 1;  // Vľavo
            break;
        case 's':
            if (*key != 0)*key = 2;  // Dole
            break;
        case 'd':
            if (*key != 1)*key = 3;  // Vpravo
            break;
        default:
            // Ignorovať neplatný vstup
            break;
        }
    }
}


