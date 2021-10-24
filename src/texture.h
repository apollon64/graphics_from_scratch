#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>
#include "upng.h"

typedef struct {
    float u;
    float v;
} tex2_t;

typedef struct {
    void* upng_handle;
    uint32_t *texels;
    int width;
    int height;
} texture_t;

texture_t load_brick_texture();
texture_t load_png_texture_data(const char* filename);
void free_png_texture(texture_t* t);
#endif
