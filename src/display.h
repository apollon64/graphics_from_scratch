#ifndef DISPLAY_H
#define DISPLAY_H

#include "typedefs.h"
#include <stdbool.h>
#include <stdint.h>

//extern SDL_Window *window;
//extern SDL_Renderer *renderer;
//extern SDL_Texture* color_buffer_texture;
extern uint32_t *color_buffer;
extern int window_width;
extern int window_height;

bool init_window(void);
void clear_color_buffer(uint32_t color);
void render_color_buffer(void);
void destroy_window(void);

#endif
