#include "texture.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "upng.h"
#include "array.h"

static int num_textures_loaded = 0;

texture_t* texture_list = NULL;

texture_t load_png_texture_data(const char* filename) {
    printf("load:%s\n",filename);
    upng_t* png_texture = upng_new_from_file(filename);
    upng_error err = upng_get_error(png_texture);
    if (err == UPNG_ENOTFOUND)
    {
        fprintf(stderr, "upng_get_error: %d\n", err);
        assert(0);
    }
    assert(upng_get_error(png_texture) == UPNG_EOK);

    if (png_texture != NULL) {
        printf("decode:%s\n", filename);
        upng_decode(png_texture);
        printf("get err:%s\n", filename);
        upng_error err = upng_get_error(png_texture);
        if (err == UPNG_EOK) {
            num_textures_loaded++;
            assert(num_textures_loaded < 256);
            texture_t texture;
            texture.handle = num_textures_loaded;
            texture.upng_handle = (void*) png_texture;
            texture.texels = (uint32_t*)upng_get_buffer(png_texture);
            texture.width = upng_get_width(png_texture);
            texture.height = upng_get_height(png_texture);
            array_push(texture_list, texture);
            return texture;
        } else {
            printf("Couldnt load texture. Probably not 32bit. upng error code: %d\n", err);
            fprintf(stderr, "upng_get_error: %d\n", err);
            assert(0);
        }
    }
    return (texture_t){};
}

void free_png_texture(texture_t* t)
{
    upng_free( (upng_t*)t->upng_handle);
    num_textures_loaded--;
    t->handle = 0;
    t->width = 0;
    t->height = 0;
    t->texels = NULL;
}

void free_all_textures()
{
    for(int i=0; i<array_length(texture_list); i++) {
        free_png_texture( &texture_list[i] );
    }
}

texture_t* find_texture_by_handle(uint8_t handle)
{
    assert(handle >= 1 && handle <= array_length(texture_list) );
    return &texture_list[handle-1]; // valid pointer as long as long as new textures are not alloced, and old are not freed
}
