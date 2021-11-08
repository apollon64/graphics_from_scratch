#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>

typedef struct {
    float u;
    float v;
} tex2_t;

typedef struct {
    uint8_t handle;
    void* upng_handle;
    uint32_t *texels;
    int width;
    int height;
} texture_t;

texture_t load_png_texture_data(const char* filename);
//void free_png_texture(texture_t* t);
void free_all_textures();
texture_t *find_texture_by_handle(uint8_t handle);
#endif
