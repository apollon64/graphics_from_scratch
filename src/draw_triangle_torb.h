#pragma once

#include <stdint.h>
#include "texture.h"
#include "vecmath.h"
#include "triangle.h"

void draw_triangle(
  float x0, float y0, float z0, float w0,
  float x1, float y1, float z1, float w1,
  float x2, float y2, float z2, float w2,
  uint32_t* colors);
depthplane_t initDepthPlane(vec4_t v[3]);
void draw_triangle_textured(vertex_texcoord_t p0, vertex_texcoord_t p1, vertex_texcoord_t p2, texture_t *texture, uint32_t* colors, float area2);
//void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
