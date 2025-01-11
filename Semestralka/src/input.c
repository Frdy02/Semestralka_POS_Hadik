#include "input.h"
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdbool.h>

void input(int* key_player1, int* key_player2) {
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

        // Spracovanie vstupu od hráča 2 (šípky)
        switch (ch) {
            case KEY_UP:
                if (*key_player2 != 2) *key_player2 = 0;  // Hore
                break;
            case KEY_LEFT:
                if (*key_player2 != 3) *key_player2 = 1;  // Vľavo
                break;
            case KEY_DOWN:
                if (*key_player2 != 0) *key_player2 = 2;  // Dole
                break;
            case KEY_RIGHT:
                if (*key_player2 != 1) *key_player2 = 3;  // Vpravo
                break;
        }
    }
}
