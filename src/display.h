#ifndef DISPLAY_H
#define DISPLAY_H

#include "typedefs.h"
#include <stdbool.h>
#include <stdint.h>

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

enum eCull_method {
    CULL_NONE,
    CULL_BACKFACE
};

enum eRender_method {
    RENDER_WIRE,
    RENDER_WIRE_VERTEX,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE
};

extern enum eCull_method cull_method;
extern enum eRender_method render_method;

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
