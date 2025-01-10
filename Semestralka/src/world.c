#include "world.h"
#include <stdio.h>
#include <string.h>

#define ZELENA "\033[1;32m"
#define ORANZOVA "\033[38;5;214m"
#define MODRA "\033[1;34m"
#define RESET_FARBA "\033[0m"

void world_init(World *world) {
    world->width = 0;
    world->height = 0;
    memset(world->grid, EMPTY, sizeof(world->grid));
}

int world_load(World *world, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Chyba pri otváraní súboru na načítanie herného sveta.");
        return -1;
    }

    if (fscanf(file, "%d %d", &world->width, &world->height) != 2) {
        fclose(file);
        fprintf(stderr, "Neplatný formát súboru! Očakáva sa: šírka/výška.\n");
        return -1;
    }

    for (int i = 0; i < world->height; i++) {
        for (int j = 0; j < world->width; j++) {
            if (i == 0 || i == world->height - 1 || j == 0 || j == world->width - 1) {
                world->grid[i][j] = WALL;
            } else {
                world->grid[i][j] = EMPTY;
            }
        }
    }

    fclose(file);
    return 0;
}

void world_draw(const World *world) {
    for (int i = 0; i < world->height; i++) {
        for (int j = 0; j < world->width; j++) {
            char c = world->grid[i][j];
            switch (c) {
                case FRUIT:
                    printf(ZELENA "%c" RESET_FARBA, c);
                    break;
                case SNAKE:
                    printf(ORANZOVA "%c" RESET_FARBA, c);
                    break;
                case WALL:
                    printf(MODRA "%c" RESET_FARBA, c);
                    break;
                default:
                    printf("%c", c);
            }
        }
        printf("\n");
    }
}


