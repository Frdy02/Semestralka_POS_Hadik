#include "input.h"
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdbool.h>

void input(int* key) {
    int ch = getch();
    if (ch == ERR) {
      return;  
    }

    flushinp();

        // Spracovanie vstupu od používateľa
        switch (ch) {
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
