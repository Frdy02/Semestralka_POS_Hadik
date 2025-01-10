#ifndef SNAKE_H
#define SNAKE_H

#include "position.h"

#define MAX_SNAKE_LENGTH 100

typedef struct {
    Position body[MAX_SNAKE_LENGTH]; // Pole pozícií pre telo hada
    int length;
    int direction;
} Snake;

void snake_init(Snake* snake, int start_x, int start_y);

#endif
