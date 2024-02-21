#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "upng.h"

int main(int argc, char* argv[])
{
  for (int i = 1; i < argc; i++)
  {
      const char* filename = argv[i];
      printf("argv[%d] %s\n", i, filename);
      upng_t* png_texture = upng_new_from_file(filename);
      upng_error err = upng_get_error(png_texture);
      if (err == UPNG_ENOTFOUND)
      {
          fprintf(stderr, "upng_get_error: %d\n", err);
          assert(0);
      }

      printf("load OK! %s\n", filename);
      if (png_texture != NULL) {
          //printf("decode upng %d, %dx%d\n", png_texture->format, png_texture->width, png_texture->height);
          upng_decode(png_texture);
          printf("get err:%s\n", filename);
          upng_error err = upng_get_error(png_texture);
          printf("err=%d\n", err);
          uint32_t* texels = (uint32_t*)upng_get_buffer(png_texture);
          printf("%u\n", texels[0]);
      }
      upng_free(png_texture);
  }

  puts("EXIT OK");
  return 0;
}
