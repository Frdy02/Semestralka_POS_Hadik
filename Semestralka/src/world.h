#ifndef WORLD_H
#define WORLD_H

#include "fruit.h"
#include "snake.h"
#include <stdbool.h>

#define MAX_WIDTH 100
#define MAX_HEIGHT 100
#define EMPTY ' ' // Prázdno
#define WALL '#'  // Stena
#define SNAKE 'O' // Had
#define FRUIT '*' // Ovocie
#define ZELENA "\033[1;32m"
#define ORANZOVA "\033[38;5;214m"
#define MODRA "\033[1;34m"
#define RESET_FARBA "\033[0m"

typedef struct {
    int width;
    int height;
    char grid[MAX_HEIGHT][MAX_WIDTH];
    Snake snake;
    FruitManager fruit_manager;
    bool game_over; // Pridaná premenná na sledovanie stavu hry
} World;

void world_init(World* world, int width, int height);
void world_update(World* world, int key);
void world_draw(const World* world);

#endif
