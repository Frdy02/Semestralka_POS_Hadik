#include "world.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void world_init(World* world, int width, int height) {
    world->width = width;
    world->height = height;
    memset(world->grid, EMPTY, sizeof(world->grid));



    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (i == 0 || i == height - 1 || j == 0 || j == width - 1) {
                world->grid[i][j] = WALL;
            }
            else {
                                world->grid[i][j] = EMPTY;
            }
        }
    }

    // Inicializácia hada
    snake_init(&world->snake, width / 2, height / 2);
    for (int i = 0; i < world->snake.length; i++) {
        world->grid[world->snake.body[i].y][world->snake.body[i].x] = SNAKE;
    }

    // Inicializácia ovocia
    fruit_manager_init(&world->fruit_manager);

        if (fruit_manager_load(&world->fruit_manager, "fruit.txt") == 0) {
                Fruit fruit;
                if (fruit_next(&world->fruit_manager, &fruit) == 0) {
                        world->grid[fruit.position.y][fruit.position.x] = FRUIT;
                }
        }

    // Inicializácia stavu hry
    world->game_over = false;
}

void world_update(World* world, int key) {
    Snake* snake = &world->snake;

    if (world->game_over) {
        return; // Ak je hra ukončená, neaktualizujeme
    }

    if (!((snake->direction == 0 && key == 2) || // Hore a dole
        (snake->direction == 2 && key == 0) || // Dole a hore
        (snake->direction == 1 && key == 3) || // Vľavo a vpravo
        (snake->direction == 3 && key == 1))) { // Vpravo a vľavo
        snake->direction = key;
    }

    // Pohyb hlavy hada
    int new_x = snake->body[0].x;
    int new_y = snake->body[0].y;

    switch (snake->direction) {
    case 0: new_y--; break; // Hore
    case 1: new_x--; break; // Vľavo
    case 2: new_y++; break; // Dole
    case 3: new_x++; break; // Vpravo
    }

    // Kontrola kolízie
    char cell = world->grid[new_y][new_x];
    if (cell == WALL || cell == SNAKE) {
        printf("Kolízia! Koniec hry.\n");
        world->game_over = true;
        return;
    }

    // Zjedol ovocie
    bool ate_fruit = (cell == FRUIT);
    if (ate_fruit) {
        snake->length++;
    }
    else {
        Position tail = snake->body[snake->length - 1];
        world->grid[tail.y][tail.x] = EMPTY;
    }




    // Posun tela hada
    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i].x = snake->body[i - 1].x;
        snake->body[i].y = snake->body[i - 1].y;
    }

    snake->body[0].x = new_x;
    snake->body[0].y = new_y;


    for (int i = 0; i < snake->length; i++) {
        world->grid[snake->body[i].y][snake->body[i].x] = SNAKE;
    }

    // Obnovenie ovocia
    Fruit fruit;
    if (ate_fruit && fruit_next(&world->fruit_manager, &fruit) == 0) {
        world->grid[fruit.position.y][fruit.position.x] = FRUIT;
    }

        world_draw(world);
}

void world_draw(const World* world) {
    printf("\033[H");
    for (int i = 0; i < world->height; i++) {
        for (int j = 0; j < world->width; j++) {
            char c = world->grid[i][j];
            switch (c) {
            case FRUIT: printf(ZELENA "%c" RESET_FARBA, c); break;
            case SNAKE: printf(ORANZOVA "%c" RESET_FARBA, c); break;
            case WALL: printf(MODRA "%c" RESET_FARBA, c); break;
            default: printf("%c", c);
            }
        }
        printf("\n");
    }
  fflush(stdout);
}
