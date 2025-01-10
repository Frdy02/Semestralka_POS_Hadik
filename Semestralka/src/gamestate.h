#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "world.h"
#include "snake.h"
#include "fruit.h"

typedef struct GameState {
    World world;         // Herný svet (mapa)
    Snake snake;         // Had
    int score;           // Aktuálne skóre
    int elapsed_time;    // Uplynulý čas v sekundách
    int game_over;       // Indikátor, či hra skončila (1 = áno, 0 = nie)
    int update_counter;
} GameState;

// Funkcie na prácu s GameState
void gamestate_init(GameState *state, const char *world_file, const char *fruit_file);
void gamestate_update(GameState *state, int direction);
void gamestate_draw(const GameState *state);
int gamestate_is_game_over(const GameState *state);
void gamestate_reset(GameState *state, const char *world_file, const char *fruit_file);

#endif


