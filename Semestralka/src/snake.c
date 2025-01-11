#include "snake.h"

void snake_init(Snake* snake, int start_x, int start_y) {
    snake->length = 1;
    snake->direction = 2; // Predvolený smer: dole

    // Inicializácia hlavy hada
    snake->body[0].x = start_x;
    snake->body[0].y = start_y;
    snake->dead = false;
}

