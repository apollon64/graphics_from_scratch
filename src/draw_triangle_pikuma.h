#pragma once

#include <stdint.h>
#include "texture.h"
#include "vector.h"
#include "triangle.h"

void draw_triangle_p(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void draw_triangle_textured_p(vertex_texcoord_t p0, vertex_texcoord_t p1, vertex_texcoord_t p2, uint32_t *texture);
void draw_filled_triangle_p(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
