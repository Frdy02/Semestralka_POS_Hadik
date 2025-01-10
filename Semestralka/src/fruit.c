#include "fruit.h"
#include <stdio.h>

void fruit_manager_init(FruitManager* manager) {
    manager->count = 0;
    manager->current_index = 0;
}

int fruit_manager_load(FruitManager* manager, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Chyba pri otvarani suboru na ovocie!\n");
        return -1;
    }

    int x;
    int y;

    while (fscanf(file, "%d %d", &x, &y) == 2) {
        manager->fruits[manager->count].position.x = x;
        manager->fruits[manager->count].position.y = y;
        printf("Nacitane ovocie: %d %d\n", x, y);
        manager->count++;
    }

    fclose(file);
    return 0;
}

int fruit_next(FruitManager* manager, Fruit* fruit) {
    if (manager->current_index >= manager->count) {
        printf("Ziadne dalsie ovocia v zozname.\n");
        return -1;
    }

    *fruit = manager->fruits[manager->current_index];
    printf("Pridane ovocie na poziciu: %d %d\n", fruit->position.x, fruit->position.y);
    manager->current_index++;
    return 0;
}
