#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vector.h"

typedef struct {
    int a;
    int b;
    int c;
} face_t;

typedef struct {
    vec2_t points[3];
    float z;
} triangle_t;

#endif
