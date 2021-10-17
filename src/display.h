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
    RENDER_FILL_TRIANGLE_WIRE,
    RENDER_TEXTURED,
    RENDER_TEXTURED_WIRE
};

extern enum eCull_method cull_method;
extern enum eRender_method render_method;

extern uint32_t *color_buffer;
extern float* z_buffer;

bool init_window(void);
void clear_color_buffer(uint32_t color);
void clear_z_buffer(float depth);
void render_color_buffer(void);
void destroy_window(void);
int get_window_width();
int get_window_height();

#endif
