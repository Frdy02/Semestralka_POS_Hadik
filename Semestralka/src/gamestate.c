#include "gamestate.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// InicializĂˇcia GameState
void gamestate_init(GameState *state, const char *world_file, const char *fruit_file) {
    // InicializĂˇcia sveta, hada a ovocia
    world_init(&state->world);
    snake_init(&state->snake, 15, 15);
    fruit_manager_init(&state->world.fruit_manager);

    // NaÄŤĂ­tanie sveta
    if (world_load(&state->world, world_file) != 0) {
        perror("Chyba pri naÄŤĂ­tanĂ­ sveta");
        state->game_over = 1;
        return;
    }

    // NaÄŤĂ­tanie ovocia
    if (fruit_manager_load(&state->world.fruit_manager, fruit_file) != 0) {
        perror("Chyba pri naÄŤĂ­tanĂ­ ovocia");
        state->game_over = 1;
        return;
    }

    // Umiestnenie hada do hernĂ©ho sveta
    for (int i = 0; i < state->snake.length; i++) {
        Position body_part = state->snake.body[i];
        state->world.grid[body_part.y][body_part.x] = SNAKE;
    }

    // Umiestnenie prvĂ©ho ovocia
    Fruit current_fruit;
    if (fruit_next(&state->world.fruit_manager, &current_fruit) == 0) {
        state->world.grid[current_fruit.y][current_fruit.x] = FRUIT;
    }

    // InicializĂˇcia skĂłre, ÄŤasu a stavu hry
    state->score = 0;
    state->elapsed_time = 0;
    state->game_over = 0;
    state->update_counter = 0;
}

// AktualizĂˇcia hernĂ©ho stavu
void gamestate_update(GameState *state, int direction) {
    if (state->game_over) return; 

    state->update_counter++;
    if (state->update_counter >= 10) {
        state->elapsed_time++;
        state->update_counter = 0;
    }

    bool ate_fruit = false;
    if (snake_move(&state->snake, &state->world, &ate_fruit, direction) != 0) {
        state->game_over = 1; // KolĂ­zia alebo neplatnĂ˝ pohyb
        return;
    }
    if (ate_fruit) {
        state->score++;
        Fruit current_fruit;
        if (fruit_next(&state->world.fruit_manager, &current_fruit) == 0) {
            state->world.grid[current_fruit.y][current_fruit.x] = FRUIT;
        }
    }
}

// Vykreslenie hernĂ©ho stavu
void gamestate_draw(const GameState *state) {
    system("clear");
    printf("Skóre: %d\n", state->score);
    printf("Čas: %d sekúnd\n", state->elapsed_time);
    world_draw(&state->world); // PouĹľitie metĂłdy na vykreslenie sveta
}

// Kontrola, ÄŤi je hra skonÄŤenĂˇ
int gamestate_is_game_over(const GameState *state) {
    return state->game_over;
}


