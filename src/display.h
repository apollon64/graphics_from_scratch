#ifndef DISPLAY_H
#define DISPLAY_H

#include "typedefs.h"
#include <stdbool.h>
#include <stdint.h>

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

extern uint32_t *color_buffer;
extern float* z_buffer;

bool open_sdl_window(void);
void alloc_framebuffer();
void pk_blit_color_to_screen(void);
void destroy_window(void);
int get_window_width();
int get_window_height();
double klock();

#endif
