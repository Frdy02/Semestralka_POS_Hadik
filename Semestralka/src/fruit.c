#include "fruit.h"

void fruit_init(Fruit* fruit) {
    fruit->position.x = -1; // Neplatná pozícia na začiatku
    fruit->position.y = -1;
}

Position fruit_get_position(const Fruit* fruit) {
    return fruit->position;
}

void fruit_set_position(Fruit* fruit, int x, int y) {
    fruit->position.x = x;
    fruit->position.y = y;
}

