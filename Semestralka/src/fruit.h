#ifndef FRUIT_H
#define FRUIT_H

#include "position.h"


typedef struct {
  Position position;

} Fruit;

void fruit_init(Fruit* fruit);
Position fruit_get_position(const Fruit* fruit);
void fruit_set_position(Fruit* fruit, int x, int y);

#endif
