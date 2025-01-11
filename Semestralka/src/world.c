#include "world.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>

void world_init_colors() {
    start_color();
    init_pair(COLOR_PAIR_FRUIT, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_PAIR_SNAKE, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_PAIR_WALL, COLOR_BLUE, COLOR_BLACK);
}

void world_generate_fruit(World* world) {
    srand(time(NULL));
    int x, y;
    do {
        x = rand() % world->width;
        y = rand() % world->height;
    } while (world->grid[y][x] != EMPTY);

    fruit_set_position(&world->fruit, x, y);
    world->grid[y][x] = FRUIT;
}

void world_init(World* world) {
    world_init_colors();
     
    world->width = 20;
    world->height = 20;

    for (int i = 0; i < world->height; i++) {
        for (int j = 0; j < world->width; j++) {
            world->grid[i][j] = (i == 0 || i == world->height - 1 || j == 0 || j == world->width - 1) ? WALL : EMPTY;
        }
    }

    snake_init(&world->snake, world->width / 2, world->height / 2);
    for (int i = 0; i < world->snake.length; i++) {
        world->grid[world->snake.body[i].y][world->snake.body[i].x] = SNAKE;
    }

    fruit_init(&world->fruit);
    world_generate_fruit(world);

    world->game_over = false;
}

void world_update(World* world, int key) {
    Snake* snake = &world->snake;

    if (world->game_over) return;

    if (!((snake->direction == 0 && key == 2) || 
          (snake->direction == 2 && key == 0) || 
          (snake->direction == 1 && key == 3) || 
          (snake->direction == 3 && key == 1))) {
        snake->direction = key;
    }

    int new_x = snake->body[0].x;
    int new_y = snake->body[0].y;

    switch (snake->direction) {
    case 0: new_y--; break;
    case 1: new_x--; break;
    case 2: new_y++; break;
    case 3: new_x++; break;
    }

    char cell = world->grid[new_y][new_x];
    if (cell == WALL || cell == SNAKE) {
        printf("Kolízia! Koniec hry.\n");
        world->game_over = true;
        return;
    }

    bool ate_fruit = (new_x == world->fruit.position.x && new_y == world->fruit.position.y);
    if (ate_fruit) {
        snake->length++;
        world_generate_fruit(world);
    } else {
        Position tail = snake->body[snake->length - 1];
        world->grid[tail.y][tail.x] = EMPTY;
    }

    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }

    snake->body[0].x = new_x;
    snake->body[0].y = new_y;

    for (int i = 0; i < snake->length; i++) {
        world->grid[snake->body[i].y][snake->body[i].x] = SNAKE;
    }

}

void world_draw(const World* world) {
    printf("%d", world->height);
    
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

