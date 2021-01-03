#include "func.h"

#include <math.h>
#include <assert.h>
#include "display.h"
#include "vector.h"
//extern uint32_t *color_buffer;
//extern int window_width;
//extern int window_height;

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


uint32_t packColor(U8 r, U8 g, U8 b)
{
    uint32_t ret = 0;
    U8 a  = 0;
    ret |= a << 24;
    ret |= r << 16;
    ret |= g << 8;
    ret |= b << 0;
    return ret;
}

void unpackColor(uint32_t c, float *r, float *g, float *b)
{
  *r = ( (c >> 16) & 0xFF) / 255.f;
  *g = ( (c >>  8) & 0xFF) / 255.f;
  *b = ( (c >>  0) & 0xFF) / 255.f;
}

uint32_t mix_colors(uint32_t a, uint32_t b, float factor)
{
  if (factor < 0) factor = 0;
  if (factor > 1) factor = 1;
  vec3_t va,vb;
  unpackColor(a, &va.x, &va.y, &va.z);
  unpackColor(b, &vb.x, &vb.y, &vb.z);
  vec3_t new_color;
  //new_color.x = lerp(va.x, vb.x, factor);
  //new_color.y = lerp(va.y, vb.y, factor);
  //new_color.z = lerp(va.z, vb.z, factor);
  new_color.x = va.x * vb.x;
  new_color.y = va.y * vb.y;
  new_color.z = va.z * vb.z;
  return packColor(255*new_color.x, 255*new_color.y, 255*new_color.z);
}

void setpix(int x, int y, uint32_t color)
{
    if (x<0) return;
    if (x>=window_width) return;
    if (y<0) return;
    if (y>=window_height) return;
    color_buffer[y*window_width+x] = color;
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
    for(int cx=startx; cx<endx; cx++)
        for(int cy=starty; cy<endy; cy++)
        {
            //if (cx<0 || cy <0 || cx>=window_width || cy >= window_height)
            //{ SDL_Log("draw_rect oob"); abort(); }
            color_buffer[cy*window_width+cx] = color;
        }
}

void draw_grid(void)
{
    int spacingX = 100;
    int spacingY = 100;
    for (size_t i = 0; i < window_width; i+=spacingX) {
        for (size_t j = 0; j < window_height; j++) {
            //setcol(127,127,127);
            setpix(i,j, packColor(75,75,75) );
        }
    }

    for (size_t y = 0; y < window_height; y+=spacingY) {
        for (size_t x = 0; x < window_width; x++) {
            //setcol(127,127,127);
            setpix(x,y, packColor(96,96,96) );
        }
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
            if (inside) setpix(xx,yy,packColor(255,255,0));
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

void draw_triangle_lines(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    draw_line(x0,y0,x1,y1,color);
    draw_line(x1,y1,x2,y2,color);
    draw_line(x2,y2,x0,y0,color);
}

void draw_flat_bottom(ivec2 a, ivec2 b, ivec2 c, uint32_t color)
{
    int x0 = a.x;
    int y0 = a.y;
    int x1 = b.x;
    int y1 = b.y;
    int x2 = c.x;
    int y2 = c.y;

    // Find the two slopes (two triangle legs)
    int height = y1 - y0;
    if (height==0) return;
    float inv_slope_1 = (float)(x1 - x0) / height;
    float inv_slope_2 = (float)(x2 - x0) / height;

    // Start x_start and x_end from the top vertex (x0,y0)
    float x_start = x0;
    float x_end = x0;

    // Loop all the scanlines from top to bottom
    for (int y = y0; y <= y2; y++) {
        draw_line(x_start, y, x_end, y, color);
        x_start += inv_slope_1;
        x_end += inv_slope_2;
    }
}

void draw_flat_top(ivec2 p0, ivec2 p1, ivec2 p2, uint32_t color)
{
    int x0 = p0.x;
    int y0 = p0.y;
    int x1 = p1.x;
    //int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;

    int height = y2 - y0;
    if (height==0) return;
    // Find the two slopes (two triangle legs)
    float inv_slope_1 = (float)(x2 - x0) / height;//(y2 - y0);
    float inv_slope_2 = (float)(x2 - x1) / height;//(y2 - y1);

    // Start x_start and x_end from the bottom vertex (x2,y2)
    float x_start = x2;
    float x_end = x2;

    // Loop all the scanlines from bottom to top
    for (int y = y2; y >= y0; y--) {
        draw_line(x_start, y, x_end, y, color);
        x_start -= inv_slope_1;
        x_end -= inv_slope_2;
    }
}
static void draw_texel(int x, int y, float u, float v, texture_t* texture)
{
  int u_clamp = (int) (u * texture->width);
  int v_clamp = (int) (v * texture->height);
  u_clamp = abs(u_clamp);
  v_clamp = abs(v_clamp);
  u_clamp %= texture->width;
  v_clamp %= texture->height;
  int tex_idx = v_clamp * texture->width + u_clamp;
  assert(tex_idx >= 0 && "tex idx less 0");
  assert(tex_idx <= texture->width*texture->height*4 && "tex idx oob");
  U8 tex_b = texture->texels[ 4*(tex_idx)+0];
  U8 tex_g = texture->texels[ 4*(tex_idx)+1];
  U8 tex_r = texture->texels[ 4*(tex_idx)+2];
  uint32_t color = 0xFFFFFFFF;
  uint32_t texel_lit = mix_colors( packColor(tex_r, tex_g, tex_b), color, .5f);
  setpix(x,y, texel_lit);
  //setpix(x,y, packColor(u*255, v*255, 0) );
}

void draw_flat_bottom_textured(vertex_texcoord_t p0, vertex_texcoord_t p1, vertex_texcoord_t p2, texture_t *texture, uint32_t color)
{
  // p0
  //p1 p2
  if(p1.x > p2.x)
  {
    vertex_texcoord_t_swap(&p1, &p2);
  }

  int x0 = p0.x;
  int y0 = p0.y;
  int x1 = p1.x;
  int y1 = p1.y;
  int x2 = p2.x;
  int y2 = p2.y;

  // Find the two slopes (two triangle legs)
  int height = y1 - y0;
  if (height==0) return;

  assert(p0.y < p1.y);
  assert(p0.y < p2.y);
  assert(p1.y==p2.y);

  float dx_dy1 = (float)(x1 - x0) / height;
  float dx_dy2 = (float)(x2 - x0) / height;

  // Loop all the scanlines from top to bottom
  for (int y = y0; y <= y1; y++) {
      float dy = y - y0;
      int x_start = x0 + dy * dx_dy1;
      int x_end = x0 + dy * dx_dy2;
      if(x_start > x_end) return;

      float tstart = (y-y0)/(float)(y1-y0);
      assert(tstart>=0.0f && tstart<=1.0f && "tt oob");
      float u_start = p0.u + tstart*(p1.u-p0.u);
      float v_start = p0.v + tstart*(p1.v-p0.v);

      float tend = (y-y0)/(float)(y2-y0);
      assert(tend>=0.0f && tend<=1.0f && "tend oob");
      float u_end = p0.u + tend*(p2.u-p0.u);
      float v_end = p0.v + tend*(p2.v-p0.v);

      for(int x=x_start; x<x_end; x++)
      {
        float tx = (x-x_start)/(float)(x_end-x_start);
        float u = lerp(u_start, u_end, tx);
        float v = lerp(v_start, v_end, tx);
        draw_texel(x, y, u, v, texture);
      }
  }
}
void draw_flat_top_textured(vertex_texcoord_t p0, vertex_texcoord_t p1, vertex_texcoord_t p2, texture_t *texture, uint32_t color)
{
  // p0 p1
  //  p2
  if(p0.x > p1.x)
  {
    vertex_texcoord_t_swap(&p0, &p1);
  }
  int x0 = p0.x;
  int y0 = p0.y;
  int x1 = p1.x;
  int y1 = p1.y;
  int x2 = p2.x;
  int y2 = p2.y;

  // Find the two slopes (two triangle legs)
  int height = y2 - y0;
  if (height==0) return;

  assert(p0.y == p1.y);
  assert(p0.y < p2.y);
  assert(p1.y < p2.y);

  float dx_dy1 = (float)(x2 - x0) / height;
  float dx_dy2 = (float)(x2 - x1) / height;

  // Loop all the scanlines from top to bottom
  for (int y = y0; y <= y2; y++) {
      float dy = y - y0;
      int x_start = x0 + dy * dx_dy1;
      int x_end = x1 + dy * dx_dy2;
      if(x_start > x_end) return;

      float tstart = (y-y0)/(float)(y2-y0);
      assert(tstart>=0.0f && tstart<=1.0f && "tt oob");
      float u_start = p0.u + tstart*(p2.u-p0.u);
      float v_start = p0.v + tstart*(p2.v-p0.v);

      float tend = (y-y0)/(float)(y2-y1);
      assert(tend>=0.0f && tend<=1.0f && "tend oob");
      float u_end = p1.u + tend*(p2.u-p1.u);
      float v_end = p1.v + tend*(p2.v-p1.v);

      for(int x=x_start; x<x_end; x++)
      {
        float tx = (x-x_start)/(float)(x_end-x_start);
        float u = lerp(u_start, u_end, tx);
        float v = lerp(v_start, v_end, tx);
        draw_texel(x, y, u, v, texture);
      }
  }
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

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t* colors)
{
    ivec2 sorted[3] = { {.x=x0,.y=y0}, {.x=x1,.y=y1}, {.x=x2,.y=y2} };

    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    if ( sorted[0].y > sorted[1].y ) {
        ivec2_swap( &sorted[0], &sorted[1] );
        uint32_t_swap( &colors[0], &colors[1] );
    }
    if ( sorted[1].y > sorted[2].y ) {
        ivec2_swap( &sorted[1], &sorted[2] );
        uint32_t_swap( &colors[1], &colors[2] );
    }
    if ( sorted[0].y > sorted[1].y ) {
        ivec2_swap( &sorted[0], &sorted[1] );
        uint32_t_swap( &colors[0], &colors[1] );
    }

    uint32_t first_color = colors[0];

    if ( sorted[1].y == sorted[2].y ) {
        draw_flat_bottom(sorted[0], sorted[1], sorted[2], first_color);
    } else if (sorted[0].y == sorted[1].y) {
        draw_flat_top(sorted[0], sorted[1], sorted[2], first_color);
    } else {
        ivec2 midpoint;
        ivec2_midpoint(sorted[0], sorted[1], sorted[2], &midpoint.x, &midpoint.y);
        draw_flat_bottom(sorted[0], sorted[1], midpoint, first_color);
        draw_flat_top(sorted[1], midpoint, sorted[2], first_color);
    }
}

void draw_triangle_textured(vertex_texcoord_t p0, vertex_texcoord_t p1, vertex_texcoord_t p2, texture_t *texture, uint32_t* colors)
{
  // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
  if ( p0.y > p1.y ) {
      vertex_texcoord_t_swap( &p0, &p1 );
  }
  if ( p1.y > p2.y ) {
      vertex_texcoord_t_swap( &p1, &p2 );
  }
  if ( p0.y > p1.y ) {
      vertex_texcoord_t_swap( &p0, &p1 );
  }

  p0.v = 1.0f - p0.v;
  p1.v = 1.0f - p1.v;
  p2.v = 1.0f - p2.v;

  int x0 = p0.x;
  int y0 = p0.y;
  int x1 = p1.x;
  int y1 = p1.y;
  int x2 = p2.x;
  int y2 = p2.y;

  uint32_t first_color = colors[0];

  if ( p1.y == p2.y ) {
      draw_flat_bottom_textured(p0, p1, p2, texture, first_color);
  } else if (p0.y == p1.y) {
      draw_flat_top_textured(p0, p1, p2, texture, first_color);
  } else {
      vertex_texcoord_t midpoint;
      float t = ivec2_midpoint(
        (ivec2){p0.x, p0.y},
        (ivec2){p1.x, p1.y},
        (ivec2){p2.x, p2.y},
        &midpoint.x, &midpoint.y
      );

      midpoint.u = p0.u + t * (p2.u - p0.u);
      midpoint.v = p0.v + t * (p2.v - p0.v);
      draw_flat_bottom_textured(p0, p1, midpoint, texture, first_color);
      draw_flat_top_textured(p1, midpoint, p2, texture, first_color);
  }
}
