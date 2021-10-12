#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <stdint.h>
#include "vector.h"

typedef struct {
    int a;
    int b;
    int c;
    int texcoord_a;
    int texcoord_b;
    int texcoord_c;
} face_t;

typedef struct {
    vec4_t points[3];
    vec2_t texcoords[3];
    uint32_t colors[3];
    vec3_t normal;
    vec3_t center;
    float z;
    float area2;
} triangle_t;



typedef struct {
    float x,y;
    float z,w;
    float u,v;
} vertex_texcoord_t;

#endif
