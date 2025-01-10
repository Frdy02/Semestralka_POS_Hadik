#ifndef FRUIT_H
#define FRUIT_H

#include "position.h"

typedef struct {
        Position position;

} Fruit;

typedef struct {
        Fruit fruits[100];
        int count;
        int current_index;

} FruitManager;

void fruit_manager_init(FruitManager* manager);
int fruit_manager_load(FruitManager* manager, const char* filename);
int fruit_next(FruitManager* manager, Fruit* fruit);

#endif
