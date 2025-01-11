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
#define OBSTACLE 'X' // Prekážka
#define COLOR_PAIR_FRUIT 1
#define COLOR_PAIR_SNAKE 2
#define COLOR_PAIR_WALL 3
#define COLOR_PAIR_OBSTACLE 4
#define MAX_PLAYERS 10

typedef struct {
    int width;
    int height;
    char** grid;
    Snake snakes[MAX_PLAYERS];
    int player_count;
    Fruit fruit;
    bool game_over; // Pridaná premenná na sledovanie stavu hry
    int typ;
    int rezim;
} World;

void world_init(World* world, int width, int height, int rezim, int typ, int playercount);
void world_update(World* world, int keys[MAX_PLAYERS]);
void world_draw(const World* world);
void world_free(World *world);

#endif
