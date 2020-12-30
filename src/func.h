#include "typedefs.h"

#include <stdbool.h>
#include <stdint.h>

int clamp(int x, int lo, int hi);
uint32_t packColor(U8 r, U8 g, U8 b);
void setpix(int x, int y, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_grid(void);
void circle(int x, int y, int r);
void line(int x0, int y0, int x1, int y1, uint32_t color);
