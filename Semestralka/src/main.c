#include "world.h"
#include "input.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> // Pre usleep

int main() {
    World world;
    int key = 2; // Predvolený smer: dole
    world_init(&world, 20, 10);

    enable_raw_mode();
    system("clear");
    while (!world.game_over) {
        input(&key);
        world_update(&world, key);
        usleep(200000); // Spomalenie hry
    }
    disable_raw_mode();
    printf("Hra skončila. Ďakujeme za hranie!\n");
    return 0;
}
