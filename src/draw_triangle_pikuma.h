#pragma once

#include <stdint.h>
#include "texture.h"
#include "vector.h"

/*
typedef struct {
    int a;
    int b;
    int c;
    tex2_t a_uv;
    tex2_t b_uv;
    tex2_t c_uv;
    uint32_t color;
} face_t;

typedef struct {
    vec2_t points[3];
    tex2_t texcoords[3];
    uint32_t color;
    float avg_depth;
} triangle_t;
*/

void draw_triangle_p(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);

void draw_filled_triangle_p(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);

void draw_textured_triangle_p(
    float x0, float y0, float z0, float w0, float u0, float v0,
    float x1, float y1, float z1, float w1, float u1, float v1,
    float x2, float y2, float z2, float w2, float u2, float v2,
    uint32_t* texture);
