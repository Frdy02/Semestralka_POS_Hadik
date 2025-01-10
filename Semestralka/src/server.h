#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include "world.h"
#include "snake.h"
#include "fruit.h"

void start_server(int port);
void handle_client(int client_sock);
void *timer_thread(void *arg);

#endif

