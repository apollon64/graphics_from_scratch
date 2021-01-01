#include "func.h"

#include <math.h>
#include <assert.h>
#include "display.h"
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

void ivec2_swap(ivec2 *a, ivec2 *b)
{
  ivec2 tmp = *a;
  *a = *b;
  *b = tmp;
}


int clamp(int x, int lo, int hi)
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
    if ( abs(x1-x0) == 0 && abs(y1-y0) == 0 ) return;
    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
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

float myRound(float v)
{
    int i = (int) v;
    if (v-i > 0.5) return i+1;
    return i;
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
        setpix( round(cx), round(cy), color);
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
  int x0 = a.x; int y0 = a.y;
  int x1 = b.x; int y1 = b.y;
  int x2 = c.x; int y2 = c.y;

  // Find the two slopes (two triangle legs)
  float inv_slope_1 = (float)(x1 - x0) / (y1 - y0);
  float inv_slope_2 = (float)(x2 - x0) / (y2 - y0);

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
  int y1 = p1.y;
  int x2 = p2.x;
  int y2 = p2.y;

  // Find the two slopes (two triangle legs)
  float inv_slope_1 = (float)(x2 - x0) / (y2 - y0);
  float inv_slope_2 = (float)(x2 - x1) / (y2 - y1);

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

ivec2 getMidpointPikuma(ivec2 p0, ivec2 p1, ivec2 p2)
{
  int mx = (((p2.x - p0.x) * (p1.y - p0.y)) / (p2.y - p0.y)) + p0.x;
  return (ivec2) {
    .x = mx,
    .y = p1.y
  };
  /*
  // Solution to the Triangle Midpoint
  ivec2 p0p1 = {p1.x - p0.x, p1.y - p0.y};
  ivec2 p0p2 = {p2.x - p0.x, p2.y - p0.y};
  float mx = p0.x + (p0p2.x*p0p1.y)/(float)p0p2.y;
  return (ivec2) {
    .x = (int) mx,
    .y = p1.y
  };
  */
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    ivec2 sorted[3] = { {.x=x0,.y=y0}, {.x=x1,.y=y1}, {.x=x2,.y=y2} };
    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    if ( sorted[0].y > sorted[1].y ) { ivec2_swap( &sorted[0], &sorted[1] ); }
    if ( sorted[1].y > sorted[2].y ) { ivec2_swap( &sorted[1], &sorted[2] ); }
    if ( sorted[0].y > sorted[1].y ) { ivec2_swap( &sorted[0], &sorted[1] ); }

    if ( sorted[1].y == sorted[2].y ) {
        draw_flat_bottom(sorted[0], sorted[1], sorted[2], color);
    } else if (sorted[0].y == sorted[1].y) {
        draw_flat_top(sorted[0], sorted[1], sorted[2], color);
    } else {
        ivec2 midpoint = getMidpointPikuma(sorted[0], sorted[1], sorted[2]);
        draw_flat_bottom(sorted[0], sorted[1], midpoint, color);
        draw_flat_top(sorted[1], midpoint, sorted[2], color);
    }

}
