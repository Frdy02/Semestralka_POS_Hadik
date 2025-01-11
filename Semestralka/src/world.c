#include "world.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>

// Funkcia na inicializáciu farieb
void world_init_colors() {
    start_color();
    init_pair(COLOR_PAIR_FRUIT, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_PAIR_SNAKE, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_PAIR_WALL, COLOR_BLUE, COLOR_BLACK);
}

// Generovanie ovocia na náhodnej pozícii
void world_generate_fruit(World* world) {
    int x, y;
    do {
        x = rand() % world->width;
        y = rand() % world->height;
    } while (world->grid[y][x] != EMPTY);

    fruit_set_position(&world->fruit, x, y);
    world->grid[y][x] = FRUIT;
}

// Inicializácia sveta
void world_init(World* world, int width, int height) {
    world->width = width;
    world->height = height;

    // Dynamická alokácia pamäte pre 2D maticu grid
    world->grid = malloc(height * sizeof(char*));
    if (!world->grid) {
        perror("Failed to allocate memory for grid");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < height; i++) {
        world->grid[i] = malloc(width * sizeof(char));
        if (!world->grid[i]) {
            perror("Failed to allocate memory for grid row");
            exit(EXIT_FAILURE);
        }
    }

    srand(time(NULL));
    world_init_colors();

    // Naplnenie gridu stenami a prázdnymi políčkami
    for (int i = 0; i < world->height; i++) {
        for (int j = 0; j < world->width; j++) {
            world->grid[i][j] = (i == 0 || i == world->height - 1 || j == 0 || j == world->width - 1) ? WALL : EMPTY;
        }
    }

    // Inicializácia hada
    snake_init(&world->snake, world->width / 2, world->height / 2);
    for (int i = 0; i < world->snake.length; i++) {
        world->grid[world->snake.body[i].y][world->snake.body[i].x] = SNAKE;
    }

    // Generovanie prvého ovocia
    fruit_init(&world->fruit);
    world_generate_fruit(world);

    world->game_over = false;
}

// Aktualizácia stavu sveta
void world_update(World* world, int key) {
    Snake* snake = &world->snake;

    if (world->game_over) return;

    // Kontrola zmeny smeru hada
    if (!((snake->direction == 0 && key == 2) || 
          (snake->direction == 2 && key == 0) || 
          (snake->direction == 1 && key == 3) || 
          (snake->direction == 3 && key == 1))) {
        snake->direction = key;
    }

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
        world->game_over = true;
        return;
    }

    // Spracovanie zjedenia ovocia
    bool ate_fruit = (new_x == world->fruit.position.x && new_y == world->fruit.position.y);
    if (ate_fruit) {
        snake->length++;
        world_generate_fruit(world);
    } else {
        Position tail = snake->body[snake->length - 1];
        world->grid[tail.y][tail.x] = EMPTY;
    }

    // Posun hada
    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }
    snake->body[0].x = new_x;
    snake->body[0].y = new_y;

    // Aktualizácia gridu s hadom
    for (int i = 0; i < snake->length; i++) {
        world->grid[snake->body[i].y][snake->body[i].x] = SNAKE;
    }
}

// Vykreslenie sveta
void world_draw(const World* world) {
    clear(); // Vyčistenie obrazovky
    for (int i = 0; i < world->height; i++) {
        for (int j = 0; j < world->width; j++) {
            char c = world->grid[i][j];
            switch (c) {
                case FRUIT:
                    attron(COLOR_PAIR(COLOR_PAIR_FRUIT));
                    mvaddch(i, j, c);
                    attroff(COLOR_PAIR(COLOR_PAIR_FRUIT));
                    break;
                case SNAKE:
                    attron(COLOR_PAIR(COLOR_PAIR_SNAKE));
                    mvaddch(i, j, c);
                    attroff(COLOR_PAIR(COLOR_PAIR_SNAKE));
                    break;
                case WALL:
                    attron(COLOR_PAIR(COLOR_PAIR_WALL));
                    mvaddch(i, j, c);
                    attroff(COLOR_PAIR(COLOR_PAIR_WALL));
                    break;
                default:
                    mvaddch(i, j, c);
            }
        }
    }
    refresh();
}

// Uvoľnenie dynamicky alokovanej pamäte
void world_free(World* world) {
    for (int i = 0; i < world->height; i++) {
        free(world->grid[i]);
    }
    free(world->grid);
}
