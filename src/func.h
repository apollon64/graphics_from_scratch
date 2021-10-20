#include "typedefs.h"

#include <stdbool.h>
#include <stdint.h>
#include "texture.h"
#include "triangle.h"

void pk_init(uint32_t *user_color_buffer, float* user_z_buffer, int xres, int yres);
void clear_color_buffer(uint32_t color);
void clear_z_buffer(float depth);
int pk_window_width();
int pk_window_height();
float* pk_z_buffer();
void int_swap(int* a, int* b);
void float_swap(float* a, float* b);
void uint32_t_swap(uint32_t* a, uint32_t* b);
void vertex_texcoord_t_swap(vertex_texcoord_t *a, vertex_texcoord_t *b);

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
