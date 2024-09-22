#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <stdint.h>
#include "vecmath.h"

typedef struct {
    float a,b,c;
    float area2;
} depthplane_t;

typedef struct {
    vec4_t points[3];
    vec2_t texcoords[3];
    uint32_t colors[3];
    float z;
    float area2;
    // uint8_t tex_hand; // We could store an index to texture per triangle to avoid segmenting tris into drawcalls
} triangle_t;

typedef struct {
    float x,y;
    float z,w;
    float u,v;
} vertex_texcoord_t;

#endif
