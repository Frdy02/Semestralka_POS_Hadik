#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdbool.h>
#include "world.h"
#include "input.h"
#include <time.h>

int main() {
    World world;
    int keys[MAX_PLAYERS] = {2, 0}; // Pole smerov pre všetkých hráčov
    bool player2_active = false;

    // Inicializácia ncurses
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);

    // Inicializácia herného sveta
    world_init(&world, 20, 20, 0, 1);

    time_t start_time = time(NULL);
    while (!world.game_over) {
        // Čas od začiatku hry
        time_t current_time = time(NULL);

        // Po 5 sekundách pridajte druhého hráča
        if (!player2_active && current_time - start_time >= 5) {
            world_add_player(&world); // Pridajte druhého hráča
            player2_active = true;
        }
        

        // Spracovanie vstupov pre všetkých hráčov
        input(&keys[0]); // keys[0] pre hráča 1, keys[1] pre hráča 2

        // Aktualizácia sveta
        world_update(&world, keys);

        // Vykreslenie sveta
        clear();
        world_draw(&world);
        refresh();

        // Spomalenie hry
        usleep(125000);
    }

    // Ukončenie hry
    endwin();
    world_free(&world);
    printf("Hra skončila. Ďakujeme za hranie!\n");
    return 0;
}
