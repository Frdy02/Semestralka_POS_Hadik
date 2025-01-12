#include "world.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>
#include <math.h>

void world_generate_obstacles(World* world) {
    int obstacle_count = (world->width * world->height) / 100; // 1% plochy
    if (obstacle_count < 2) {
        obstacle_count = 2; // Minimálne 2 prekážky
    }
 
    // Definované bezpečné zóny pre hráčov (o 3 bunky od okrajov)
    int safe_positions[5][2] = {
        {world->width / 2, world->height / 2},           // Stred
        {3, 3},                                          // Horný ľavý roh (posunutý)
        {world->width - 4, 3},                           // Horný pravý roh (posunutý)
        {3, world->height - 4},                          // Dolný ľavý roh (posunutý)
        {world->width - 4, world->height - 4}            // Dolný pravý roh (posunutý)
    };
 
    for (int i = 0; i < obstacle_count; i++) {
        int x, y, valid_position = 0;
 
        // Nájdite voľnú pozíciu mimo bezpečných zón
        while (!valid_position) {
            x = rand() % (world->width - 2) + 1;  // Vyhnite sa stenám
            y = rand() % (world->height - 2) + 1;
 
            // Skontrolujte, či pozícia nie je v bezpečných zónach
            valid_position = 1;
            for (int j = 0; j < 5; j++) {
                if (x == safe_positions[j][0] && y == safe_positions[j][1]) {
                    valid_position = 0;
                    break;
                }
            }
 
            // Skontrolujte, či je pozícia prázdna
            if (valid_position && world->grid[y][x] != EMPTY) {
                valid_position = 0;
            }
        }
 
        // Umiestnite prekážku
        world->grid[y][x] = OBSTACLE;
    }
}
 
 
 
 
 
void world_add_player(World* world) {
    if (world->player_count >= MAX_PLAYERS) {
        printf("Maximálny počet hráčov dosiahnutý.\n");
        return;
    }
 
    int player_id = world->player_count;
    Snake* snake = &world->snakes[player_id];
 
    // Definované pozície na spawnovanie hráčov (o 3 miesta od okrajov)
    int spawn_positions[5][2] = {
        {world->width / 2, world->height / 2},            // Stred
        {3, 3},                                           // Horný ľavý roh (posunutý)
        {world->width - 4, 3},                            // Horný pravý roh (posunutý)
        {3, world->height - 4},                           // Dolný ľavý roh (posunutý)
        {world->width - 4, world->height - 4}             // Dolný pravý roh (posunutý)
    };
 
    bool safe = false;
    int x, y;
 
    // Nájdite bezpečnú pozíciu
    for (int i = 0; i < 5; i++) {
        x = spawn_positions[i][0];
        y = spawn_positions[i][1];
 
        safe = true;
 
        // Skontrolujte, či sa tam nenachádza iný hráč
        for (int j = 0; j < world->player_count; j++) {
            Snake* other_snake = &world->snakes[j];
            for (int k = 0; k < other_snake->length; k++) {
                if (other_snake->body[k].x == x && other_snake->body[k].y == y) {
                    safe = false;
                    break;
                }
            }
            if (!safe) break;
        }
 
        // Skontrolujte, či sa tam nenachádza prekážka
        if (world->grid[y][x] != EMPTY) {
            safe = false;
        }
 
        if (safe) break; // Ak sme našli bezpečné miesto, ukončíme hľadanie
    }
 
    // Ak nebola nájdená bezpečná predvolená pozícia, generujeme náhodne
    if (!safe) {
        do {
            x = rand() % (world->width - 2) + 1;
            y = rand() % (world->height - 2);
 
            safe = true;
 
            // Rovnaké kontroly pre náhodnú pozíciu
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
    }
 
    // Inicializujte hada
    snake_init(snake, x, y);
    snake->id = player_id;
 
    // Označte hada na mriežke
    for (int i = 0; i < snake->length; i++) {
        world->grid[snake->body[i].y][snake->body[i].x] = SNAKE;
    }
 
    world->player_count++;
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
void world_init(World* world, int width, int height, int rezim, int typ) {
    world->player_count = 0;
    world->width = width;
    world->height = height;
    world->typ = typ;
    world->rezim = rezim;

    // Dynamická alokácia pamäte pre grid
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
    

    // Inicializácia mriežky (steny a prázdne políčka)
    for (int i = 0; i < world->height; i++) {           // Prechádza riadkami
        for (int j = 0; j < world->width; j++) {        // Prechádza stĺpcami
            world->grid[i][j] = (i == 0 || i == world->height - 1 || j == 0 || j == world->width - 1) ? WALL : EMPTY;
    }
}


    // Ak je typ sveta 2, generuj prekážky
    if (typ == 2) {
        world_generate_obstacles(world);
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

        // Wrap-around logika pre typ sveta 1
        if (world->typ == 1) {
            if (new_x < 0) new_x = world->width - 2;        // Zľava na pravú stranu (vyhnutie sa stene)
            if (new_x >= world->width - 1) new_x = 1;      // Sprava na ľavú stranu (vyhnutie sa stene)
            if (new_y < 0) new_y = world->height - 2;      // Zhora na spodnú stranu (vyhnutie sa stene)
            if (new_y >= world->height - 1) new_y = 1;     // Zospodu na hornú stranu (vyhnutie sa stene)
        } else { // Typ sveta 2: Kontrola kolízie so stenami
            if (new_x <= 0 || new_x >= world->width - 1 || new_y <= 0 || new_y >= world->height - 1) {
                mvprintw(world->height + 4, 0, "Hráč %d narazil do steny!", p);
                snake->dead = true;
                continue;
            }
        }

        // Kontrola kolízie s vlastným telom
        for (int i = 1; i < snake->length; i++) {
            if (snake->body[i].x == new_x && snake->body[i].y == new_y) {
                mvprintw(world->height + 3, 0, "Hráč %d narazil do svojho tela!", p);
                snake->dead = true;
                break;
            }
        }

        // Kontrola kolízie s prekážkami (iba ak typ je 2)
        if (world->typ == 2 && world->grid[new_y][new_x] == OBSTACLE) {
            mvprintw(world->height + 2, 0, "Hráč %d narazil do prekážky!", p);
            snake->dead = true;
            continue;
        }

        if (snake->dead) continue;

        // Kontrola kolízie s inými hadmi
        for (int i = 0; i < world->player_count; i++) {
            if (i == p) continue; // Preskočte aktuálneho hráča
            Snake* other_snake = &world->snakes[i];
            for (int j = 0; j < other_snake->length; j++) {
                if (other_snake->body[j].x == new_x && other_snake->body[j].y == new_y) {
                    mvprintw(world->height + 1, 0, "Hráč %d narazil do hráča %d!", p, i);
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
            mvprintw(world->height + 30, 0, "Hráč %d zjedol ovocie!", p);
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

    // Obnovenie stien po wrap-around
    for (int i = 0; i < world->width; i++) {
        world->grid[0][i] = WALL;                      // Horná stena
        world->grid[world->height - 1][i] = WALL;      // Dolná stena
    }
    for (int i = 0; i < world->height; i++) {
        world->grid[i][0] = WALL;                      // Ľavá stena
        world->grid[i][world->width - 1] = WALL;       // Pravá stena
    }

    // Vyčistite telá mŕtvych hadov
    for (int p = 0; p < world->player_count; p++) {
        Snake* snake = &world->snakes[p];
        if (snake->dead && snake->length > 0) {
            usleep(200000); // Delay pred čistením
            for (int i = 0; i < snake->length; i++) {
                world->grid[snake->body[i].y][snake->body[i].x] = EMPTY;
            }
            snake->length = 0; // Reset dĺžky mŕtveho hada
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


// Uvoľnenie dynamicky alokovanej pamäte
void world_free(World* world) {
    for (int i = 0; i < world->height; i++) {
        free(world->grid[i]);
    }
    free(world->grid);
}
