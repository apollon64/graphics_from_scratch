#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>
#include "vecmath.h"

typedef struct {
    vec3_t direction;
    vec3_t position;
    vec4_t position_proj;
} light_t;

extern light_t light;

uint32_t light_apply_intensity(uint32_t original_color, float percentage_factor);

#endif
