#include "display.h"

#include <SDL2/SDL.h>

static int window_width;
static int window_height;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture* color_buffer_texture = NULL;
uint32_t *color_buffer = NULL;
float *z_buffer = NULL;

bool init_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("Error initing SDL.\n");
        return false;
    }

    // Set size of SDL window to screen size
    SDL_DisplayMode display_mode;
    SDL_GetDisplayMode(0, 0, &display_mode);
    int fullscreen_width = 800;//display_mode.w;
    int fullscreen_height = 600;//display_mode.h;
    window_width = fullscreen_width;
    window_height = fullscreen_height;
    SDL_Log("set window %dx%d\n", fullscreen_width, fullscreen_height);

    int posX=SDL_WINDOWPOS_CENTERED;
    int posY=SDL_WINDOWPOS_CENTERED;
    window = SDL_CreateWindow(
                 NULL /*border*/,
                 posX,
                 posY,
                 fullscreen_width,
                 fullscreen_height,
                 SDL_WINDOW_BORDERLESS /*flags*/
             );
    if (!window) {
        SDL_Log("Error creating SDL window.\n");
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, 0 /*flags*/);
    if (!renderer) {
        SDL_Log("Error creating SDL renderer.\n");
        return false;
    }

    int c_bytes = sizeof(uint32_t) * window_width * window_height;
    int z_bytes = sizeof(float) * window_width * window_height;
    color_buffer = (uint32_t*)malloc(c_bytes);
    z_buffer = (float*)malloc(z_bytes);

    color_buffer_texture = SDL_CreateTexture(
                               renderer,
                               SDL_PIXELFORMAT_RGBA32,
                               SDL_TEXTUREACCESS_STREAMING,
                               window_width,
                               window_height
                           );

    return true;
}

static uint32_t packColor(U8 r, U8 g, U8 b)
{
    uint32_t ret = 0xFF000000;
    //U8 a = 0;
    //ret |= a << 24;
    ret |= b << 16;
    ret |= g << 8;
    ret |= r << 0;
    return ret;
}

void render_color_buffer(void) {

    // Display Z buffer as color
//    for(int j=0; j<window_height; j++)
//    for(int i=0; i<window_width; i++)
//    {
//        U8 z = (U8)( 255*z_buffer[window_width*j+i] );
//        color_buffer[window_width*j+i] = packColor(z,z,z);
//    }


    SDL_UpdateTexture(
        color_buffer_texture,
        NULL, // rect
        color_buffer, // pixels
        (int)(window_width * sizeof(uint32_t) ) // bytes per row, pitch
    );

    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void destroy_window(void)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int get_window_width()
{
    return window_width;
}
int get_window_height()
{
    return window_height;
}
