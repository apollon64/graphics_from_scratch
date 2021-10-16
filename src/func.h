#include "typedefs.h"

#include <stdbool.h>
#include <stdint.h>
#include "texture.h"
#include "triangle.h"

float lerp (float a, float b, float f);
int clamp(int x, int lo, int hi);
uint32_t packColor(U8 r, U8 g, U8 b);
void unpackColor(uint32_t c, float *r, float *g, float *b);
void setpix(int x, int y, uint32_t color);
void setpix_no_bound_check(int x, int y, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_grid(void);
void circle(int x, int y, int r);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_line3d(int x0, int y0, float z0, int x1, int y1, float z1, uint32_t color);
void draw_line_dda(int x0, int y0, int x1, int y1, uint32_t color);
void draw_triangle_lines(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
