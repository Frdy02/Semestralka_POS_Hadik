#ifndef WORLD_H
#define WORLD_H


#include "snake.h"
#include "fruit.h"
#include <stdbool.h>
#include <ncurses.h>

#define EMPTY ' ' // Prázdno
#define WALL '#'  // Stena
#define SNAKE 'O' // Had
#define FRUIT '*' // Ovocie
#define COLOR_PAIR_FRUIT 1
#define COLOR_PAIR_SNAKE 2
#define COLOR_PAIR_WALL 3

typedef struct {
    int width;
    int height;
    char grid[20][20];
    Snake snake;
    Fruit fruit;
    bool game_over; // Pridaná premenná na sledovanie stavu hry
} World;

void world_init(World* world);
void world_update(World* world, int key);
void world_draw(const World* world);

#endif
