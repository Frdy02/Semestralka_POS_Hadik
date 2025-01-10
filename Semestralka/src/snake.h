#ifndef SNAKE_H
#define SNAKE_H

#include <stdbool.h>
#include "common.h"
#include "world.h"

#define MAX_SNAKE_LENGTH 100

typedef struct {
  Position body[MAX_SNAKE_LENGTH];
  int length;
  int direction;
} Snake;

void snake_init(Snake *snake, int start_x, int start_y);
void snake_set_direction(Snake *snake, int key);
int snake_move(Snake *snake, World *world, bool *ate_fruit, int key);
bool snake_check_collision(const Snake *snake);

#endif

