#include "world.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>
#include <math.h>

void world_add_player(World* world) {
    bool safe;
    if (world->player_count >= MAX_PLAYERS) {
        printf("Maximálny počet hráčov dosiahnutý.\n");
        return;
    }

    int player_id = world->player_count;
    Snake* snake = &world->snakes[player_id];
    int x, y;

    // Nájdite bezpečný bod pre spawn
    do {
        x = rand() % (world->width - 2) + 1;  // Vyhnite sa stenám
        y = rand() % (world->height - 2) + 1;

        safe = true;

        // Skontrolujte, či je bod bezpečný
        for (int i = 0; i < world->player_count; i++) {
            Snake* other_snake = &world->snakes[i];
            for (int j = 0; j < other_snake->length; j++) {
                if (other_snake->body[j].x == x && other_snake->body[j].y == y) {
                    safe = false;
                    break;
                }
            }
            if (!safe) break;
        }

        if (world->grid[y][x] != EMPTY) {
            safe = false;
        }

    } while (!safe);

    // Inicializujte hada
    snake_init(snake, x, y);
    snake->id = player_id;

    // Označte hada na mriežke
    for (int i = 0; i < snake->length; i++) {
        world->grid[snake->body[i].y][snake->body[i].x] = SNAKE;
    }

    world->player_count++;
}


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
void world_init(World* world, int width, int height, int rezim, int typ, int playercount) {
    world->player_count = playercount;
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

    // Inicializácia hada STATICKY
    snake_init(&world->snakes[0], world->width / 2, world->height / 2);
    for (int i = 0; i < world->snakes[0].length; i++) {
        world->grid[world->snakes[0].body[i].y][world->snakes[0].body[i].x] = SNAKE;
    }

    // Generovanie prvého ovocia
    fruit_init(&world->fruit);
    world_generate_fruit(world);

    world->game_over = false;
}

// Aktualizácia stavu sveta
void world_update(World* world, int keys[MAX_PLAYERS]) {
    // Iterujte cez všetkých hráčov
    for (int p = 0; p < world->player_count; p++) {
        Snake* snake = &world->snakes[p];

        // Preskočte mŕtve hady
        if (snake->dead) continue;

        // Aktualizujte smer hada podľa aktuálneho vstupu
        if (!((snake->direction == 0 && keys[p] == 2) || 
              (snake->direction == 2 && keys[p] == 0) || 
              (snake->direction == 1 && keys[p] == 3) || 
              (snake->direction == 3 && keys[p] == 1))) {
            snake->direction = keys[p];
        }

        int new_x = snake->body[0].x;
        int new_y = snake->body[0].y;

        // Vypočítajte nový smer
        switch (snake->direction) {
            case 0: new_y--; break; // Hore
            case 1: new_x--; break; // Vľavo
            case 2: new_y++; break; // Dole
            case 3: new_x++; break; // Vpravo
        }

        // Kontrola kolízie so stenami
        if (new_x <= 0 || new_x >= world->width - 1 || new_y <= 0 || new_y >= world->height - 1) {
            printf("Hráč %d narazil do steny!\n", p);
            snake->dead = true;
            continue;
        }

        // Kontrola kolízie s vlastným telom
        for (int i = 1; i < snake->length; i++) {
            if (snake->body[i].x == new_x && snake->body[i].y == new_y) {
                printf("Hráč %d narazil do svojho tela!\n", p);
                snake->dead = true;
                break;
            }
        }
        if (snake->dead) continue;

        // Kontrola kolízie s inými hadmi
        for (int i = 0; i < world->player_count; i++) {
            if (i == p) continue; // Preskočte aktuálneho hráča
            Snake* other_snake = &world->snakes[i];
            for (int j = 0; j < other_snake->length; j++) {
                if (other_snake->body[j].x == new_x && other_snake->body[j].y == new_y) {
                    printf("Hráč %d narazil do hráča %d!\n", p, i);
                    snake->dead = true;
                    break;
                }
            }
            if (snake->dead) break;
        }
        if (snake->dead) continue;

        // Kontrola zjedenia ovocia
        bool ate_fruit = (new_x == world->fruit.position.x && new_y == world->fruit.position.y);
        if (ate_fruit) {
            snake->length++;
            printf("Hráč %d zjedol ovocie!\n", p);
            world_generate_fruit(world);
        } else {
            // Uvoľnite poslednú pozíciu hada
            Position tail = snake->body[snake->length - 1];
            world->grid[tail.y][tail.x] = EMPTY;
        }

        // Posun tela hada
        for (int i = snake->length - 1; i > 0; i--) {
            snake->body[i] = snake->body[i - 1];
        }
        snake->body[0].x = new_x;
        snake->body[0].y = new_y;

        // Aktualizujte pozície hada na mriežke
        for (int i = 0; i < snake->length; i++) {
            world->grid[snake->body[i].y][snake->body[i].x] = SNAKE;
        }
    }

    // Skontrolujte, či sú všetci hráči mŕtvi
    bool all_dead = true;
    for (int i = 0; i < world->player_count; i++) {
        if (!world->snakes[i].dead) {
            all_dead = false;
            break;
        }
    }
    if (all_dead) {
        printf("Všetci hráči sú mŕtvi. Hra končí!\n");
        world->game_over = true;
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
