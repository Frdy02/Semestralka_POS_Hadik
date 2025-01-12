#include "input.h"
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdbool.h>

void input(int* key_player1) {
    int ch;

    // Spracovanie vstupu v cykle
    while ((ch = getch()) != ERR) {
        // Spracovanie vstupu od hráča 1 (WASD)
        switch (ch) {
            case 'w':
                if (*key_player1 != 2) *key_player1 = 0;  // Hore
                break;
            case 'a':
                if (*key_player1 != 3) *key_player1 = 1;  // Vľavo
                break;
            case 's':
                if (*key_player1 != 0) *key_player1 = 2;  // Dole
                break;
            case 'd':
                if (*key_player1 != 1) *key_player1 = 3;  // Vpravo
                break;
        }

    }
}

