#include "world.h"
#include "input.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> // Pre usleep
#include <ncurses.h>

int main() {
    World world;
    int key = 2; // Predvolený smer: dole
    
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);
    
    world_init(&world);
    
    while (!world.game_over) {
        input(&key);
        world_update(&world, key);
        usleep(200000); // Spomalenie hry
    }
    endwin();
    printf("Hra skončila. Ďakujeme za hranie!\n");
    return 0;
}
