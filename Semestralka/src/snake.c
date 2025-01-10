#include "snake.h"
#include <unistd.h>

void snake_init(Snake *snake, int start_x, int start_y) {
    snake->length = 1;
    snake->direction = 2; // Predvolený smer: dole

    for (int i = 0; i < snake->length; i++) {
        snake->body[i].x = start_x - i;
        snake->body[i].y = start_y;
    }
}

int snake_move(Snake *snake, World *world, bool *ate_fruit, int key) {
    Position new_head = snake->body[0];

    // Nepovoľ spätný pohyb pre hada dlhšieho ako 1
    if (snake->length > 1 &&
        ((snake->direction == 0 && key == 2) || // Hore a dole
         (snake->direction == 2 && key == 0) || // Dole a hore
         (snake->direction == 1 && key == 3) || // Vľavo a vpravo
         (snake->direction == 3 && key == 1))) { // Vpravo a vľavo
        key = snake->direction; // Ignoruj spätný pohyb
    }

    // Aktualizácia smeru
    snake->direction = key;

    // Posun hlavy na nový smer
    switch (key) {
        case 0: new_head.y--; break; // Hore
        case 1: new_head.x--; break; // Vľavo
        case 2: new_head.y++; break; // Dole
        case 3: new_head.x++; break; // Vpravo
        default: return -1; // Neplatný smer
    }

    // Kontrola kolízie
    if (world->grid[new_head.y][new_head.x] == WALL ||
        world->grid[new_head.y][new_head.x] == SNAKE) {
        return -1; // Kolízia s prekážkou alebo vlastným telom
    }

    *ate_fruit = (world->grid[new_head.y][new_head.x] == FRUIT);

    // Odstránenie chvosta hada z gridu, ak nezjedol ovocie
    if (!*ate_fruit) {
        Position tail = snake->body[snake->length - 1];
        world->grid[tail.y][tail.x] = EMPTY;
    } else {
        snake->length++; // Had rastie
    }

    // Posunutie tela hada
    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }
    snake->body[0] = new_head;

    // Obnovenie hada do sveta
    for (int i = 0; i < snake->length; i++) {
        Position body_part = snake->body[i];
        world->grid[body_part.y][body_part.x] = SNAKE;
    }
    return 0;
}

bool snake_check_collision(const Snake *snake) {
    Position head = snake->body[0];

    for (int i = 1; i < snake->length; i++) {
        if (snake->body[i].x == head.x && snake->body[i].y == head.y) {
            return true; // Kolízia s vlastným telom
        }
    }
    return false;
}


