#ifndef WORLD_H
#define WORLD_H

#include "common.h"
#include "fruit.h"

#define MAX_WIDTH 100
#define MAX_HEIGHT 100

typedef struct {
  int width;
  int height;
  char grid[MAX_HEIGHT][MAX_WIDTH];
  FruitManager fruit_manager;
} World;

void world_init(World *world);
int world_load (World *world, const  char *filename);
void world_draw(const World *world);

#endif

