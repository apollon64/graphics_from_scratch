#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>
#include "upng.h"

typedef struct {
    float u;
    float v;
} tex2_t;

typedef struct {
  uint8_t *texels;
  int width;
  int height;
} texture_t;

extern int texture_width;
extern int texture_height;
extern uint32_t* mesh_texture;
extern const uint8_t REDBRICK_TEXTURE[];

void load_png_texture_data(const char* filename);
void free_png_texture();
#endif
