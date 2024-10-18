#ifndef DISPLAY_H
#define DISPLAY_H


#include <stdint.h>
#include <stdbool.h>

int init(int window_width, int window_height);
bool dopoll();
void display(int width, int height, uint8_t *grid1, uint8_t *grid2, uint8_t *grid3);
void deinit();


#endif
