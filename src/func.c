#include "func.h"

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "vecmath.h"
#include "draw_triangle_torb.h"


static uint32_t *color_buffer;
static float* z_buffer;
static int window_width;
static int window_height;

void pk_init(uint32_t *user_color_buffer, float* user_z_buffer, int xres, int yres)
{
    color_buffer = user_color_buffer;
    z_buffer = user_z_buffer;
    window_width = xres;
    window_height = yres;
}
void clear_color_buffer(uint32_t color) {
    size_t count = window_width * window_height;
    for (size_t i = 0; i < count; i++) {
        color_buffer[i] = color;
    }
}
void clear_z_buffer(float depth) {
    size_t count = window_width * window_height;
    for (size_t i = 0; i < count; i++) {
        z_buffer[i] = depth;
    }
}

int pk_window_width()
{
    return window_width;
}
int pk_window_height()
{
    return window_height;
}
float* pk_z_buffer()
{
    return z_buffer;
}

typedef struct
{
    int x,y;
} ivec2;

void int_swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void float_swap(float* a, float* b) {
    float tmp = *a;
    *a = *b;
    *b = tmp;
}


void uint32_t_swap(uint32_t* a, uint32_t* b) {
    uint32_t tmp = *a;
    *a = *b;
    *b = tmp;
}

void ivec2_swap(ivec2 *a, ivec2 *b)
{
    ivec2 tmp = *a;
    *a = *b;
    *b = tmp;
}

void vertex_texcoord_t_swap(vertex_texcoord_t *a, vertex_texcoord_t *b)
{
    vertex_texcoord_t tmp = *a;
    *a = *b;
    *b = tmp;
}

float lerp (float a, float b, float f) {
    return (a * (1.0f - f)) + (b * f);
}

int clamp(int x, int lo, int hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

float clampf(float x, float lo, float hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

uint32_t packColorRGBf(float r, float g, float b)
{
    r = clampf(r, 0.0, 1.0);
    g = clampf(g, 0.0, 1.0);
    b = clampf(b, 0.0, 1.0);
    uint32_t ret = 0xFF000000;
    //U8 a = 0;
    //ret |= a << 24;
    ret |= (U8)(b*255) << 16;
    ret |= (U8)(g*255) << 8;
    ret |= (U8)(r*255) << 0;
    return ret;
}

uint32_t packColorRGBAf(float r, float g, float b, float a)
{
    r = clampf(r, 0.0, 1.0);
    g = clampf(g, 0.0, 1.0);
    b = clampf(b, 0.0, 1.0);
    a = clampf(a, 0.0, 1.0);
    uint32_t ret = 0;
    ret |= (U8)(a*255) << 24;
    ret |= (U8)(b*255) << 16;
    ret |= (U8)(g*255) << 8;
    ret |= (U8)(r*255) << 0;
    return ret;
}

uint32_t packColorRGB(U8 r, U8 g, U8 b)
{
    uint32_t ret = 0xFF000000;
    //U8 a = 0;
    //ret |= a << 24;
    ret |= b << 16;
    ret |= g << 8;
    ret |= r << 0;
    return ret;
}

uint32_t packColorRGBA(U8 r, U8 g, U8 b, U8 a)
{
    uint32_t ret = 0;
    ret |= a << 24;
    ret |= b << 16;
    ret |= g << 8;
    ret |= r << 0;
    return ret;
}

void unpackColorRGBf(uint32_t c, float *r, float *g, float *b)
{
    *b = ( (c >> 16) & 0xFF) / 255.f;
    *g = ( (c >>  8) & 0xFF) / 255.f;
    *r = ( (c >>  0) & 0xFF) / 255.f;
}

void unpackColorRGBAf(uint32_t c, float *r, float *g, float *b, float *a)
{
    *a = ( (c >> 24) & 0xFF) / 255.f;
    *b = ( (c >> 16) & 0xFF) / 255.f;
    *g = ( (c >>  8) & 0xFF) / 255.f;
    *r = ( (c >>  0) & 0xFF) / 255.f;
}

void unpackColorRGB(uint32_t c, U8 *r, U8 *g, U8 *b)
{
    *b = ( (c >> 16) & 0xFF) ;
    *g = ( (c >>  8) & 0xFF) ;
    *r = ( (c >>  0) & 0xFF) ;
}

void unpackColorRGBA(uint32_t c, U8 *r, U8 *g, U8 *b, U8 *a)
{
    *a = ( (c >> 24) & 0xFF) ;
    *b = ( (c >> 16) & 0xFF) ;
    *g = ( (c >>  8) & 0xFF) ;
    *r = ( (c >>  0) & 0xFF) ;
}


uint32_t mix_colors(uint32_t a, uint32_t b, float factor)
{
    if (factor < 0) factor = 0;
    if (factor > 1) factor = 1;
    vec3_t va,vb;
    unpackColorRGBf(a, &va.x, &va.y, &va.z);
    unpackColorRGBf(b, &vb.x, &vb.y, &vb.z);
    vec3_t new_color;
    //new_color.x = lerp(va.x, vb.x, factor);
    //new_color.y = lerp(va.y, vb.y, factor);
    //new_color.z = lerp(va.z, vb.z, factor);
    new_color.x = va.x * vb.x;
    new_color.y = va.y * vb.y;
    new_color.z = va.z * vb.z;
    return packColorRGB(255*new_color.x, 255*new_color.y, 255*new_color.z);
}

void setpix(int x, int y, uint32_t color)
{
    if (x<0) return;
    if (x>=window_width) return;
    if (y<0) return;
    if (y>=window_height) return;
    color_buffer[y*window_width+x] = color;
}

void setpix_no_bound_check(int x, int y, uint32_t color)
{
    color_buffer[y*window_width+x] = color;
}

uint32_t getpix(int x, int y)
{
    if (x<0) return 0;
    if (x>=window_width) return 0;
    if (y<0) return 0;
    if (y>=window_height) return 0;
    return color_buffer[y*window_width+x];
}

void draw_rect(int x, int y, int width, int height, uint32_t color)
{
    if (width < 1 || height < 1) return;
    if (x >= window_width || y >= window_height) return;
    int endx = x+width;
    int endy = y+height;
    if (endx <= 0 || endy <= 0) return;
    int startx = clamp(x, 0, window_width-1);
    int starty = clamp(y, 0, window_height-1);
    endx = clamp(endx, 0, window_width-1);
    endy = clamp(endy, 0, window_height-1);
    for (int cy = starty; cy < endy; cy++)
        for (int cx = startx; cx < endx; cx++)
        {
            //if (cx<0 || cy <0 || cx>=window_width || cy >= window_height)
            //{ SDL_Log("draw_rect oob"); abort(); }
            color_buffer[cy*window_width+cx] = color;
        }
}

void circle(int x, int y, int r)
{
    if(r < 1) return;

    for(int i=-r; i<=r; i++)
    {
        for(int j=-r; j<=r; j++)
        {
            int xx =x + i;
            int yy =y + j;

            int dx = xx-x;
            int dy = yy-y;
            bool inside = (dx*dx + dy*dy) < r*r;
            if (inside) setpix(xx,yy,packColorRGB(255,255,0));
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
    int dx = x1-x0;
    int dy = y1-y0;
    if ( abs(dx) == 0 && abs(dy) == 0 ) return;
    if ( abs(dx) > 1e6 && abs(dy) > 1e6 ) return;
    dx = abs(x1-x0);
    dy = abs(y1-y0);
    int sx = x0<x1 ? 1 : -1;
    int sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;

    for(;;) {
        setpix(x0,y0,color);
        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy) {
            err += dx;
            y0 += sy;
        }
    }
}

void draw_line3d(int x0, int y0, float w0, int x1, int y1, float w1, uint32_t color)
{
    if (x0 < 0 && x1 < 0) return;
    if (y0 < 0 && y1 < 0) return;
    if (x0 > window_width && x1 > window_width) return;
    if (y0 > window_height && y1 > window_height) return;
  int delta_x = x1-x0;
  int delta_y = y1-y0;
  int delta_reciprocal_w = 1.f/w1 - 1.f/w0;
  if ( abs(delta_x) == 0 && abs(delta_y) == 0 ) return;

  int side_len = abs(delta_x) > abs(delta_y) ? abs(delta_x) : abs(delta_y);
  float x_inc = delta_x / (float)side_len;
  float y_inc = delta_y / (float)side_len;
  float w_inc = delta_reciprocal_w / (float)side_len;
  float cx = x0;
  float cy = y0;
  float one_over_w = 1.0f / w0;
  for(int i=0; i<=side_len; i++) {
      int x = roundf(cx);
      int y = roundf(cy);


      float interpolated_z;
      // Also interpolate the value of 1/w for the current pixel
      interpolated_z = 1.0f - one_over_w;

      // One way would be do lerp inverse W every pixel. 2 mul, 2 add/sub
      //interpolated_z = 1.0f - lerp( 1.f/w0, 1.f/w1, i/(float)side_len );

      if (x >= 0 && y >=0 && x < window_width && y < window_height)
      {
          if (interpolated_z < z_buffer[y*window_width+x])
          {
             setpix( x, y, color);
             z_buffer[y*window_width+x] = interpolated_z;
          }
      }

      cx += x_inc;
      cy += y_inc;
      one_over_w += w_inc;
  }
}


void draw_line_dda(int x0, int y0, int x1, int y1, uint32_t color)
{
    int delta_x = x1-x0;
    int delta_y = y1-y0;
    if ( abs(delta_x) == 0 && abs(delta_y) == 0 ) return;

    int side_len = abs(delta_x) > abs(delta_y) ? abs(delta_x) : abs(delta_y);
    float x_inc = delta_x / (float)side_len;
    float y_inc = delta_y / (float)side_len;
    float cx = x0;
    float cy = y0;
    for(int i=0; i<=side_len; i++) {
        setpix( roundf(cx), roundf(cy), color);
        cx += x_inc;
        cy += y_inc;
    }
}

void draw_triangle_lines(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    draw_line(x0,y0,x1,y1,color);
    draw_line(x1,y1,x2,y2,color);
    draw_line(x2,y2,x0,y0,color);
}


float ivec2_midpoint( ivec2 p0, ivec2 p1, ivec2 p2, int *x, int *y)
{
    // Trivially know that my is p1.y
    int my = p1.y;
    // Vector from top point to bottom point
    vec2_t p0p2 = {p2.x - p0.x, p2.y - p0.y};
    // p0 + t * p0p2 = m
    // Solve for interpolation value t
    float t = (p1.y - p0.y) / p0p2.y;
    int mx = (int)(p0.x + t*p0p2.x);
    *x = mx;
    *y = my;
    return t;
}
