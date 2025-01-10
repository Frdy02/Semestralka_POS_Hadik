#ifndef INPUT_H
#define INPUT_H

#include <ncurses.h>

void enable_raw_mode();
void disable_raw_mode();
int kbhit(void);
char get_input(void);
void input(int* key);

#endif
