#include "func.h"

#include <math.h>
#include "display.h"
//extern uint32_t *color_buffer;
//extern int window_width;
//extern int window_height;

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

void draw_triangle_lines(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    draw_line(x0,y0,x1,y1,color);
    draw_line(x1,y1,x2,y2,color);
    draw_line(x2,y2,x0,y0,color);
}

typedef struct
{
  int x,y;
} ivec2;

void ivec2_swap(ivec2 *a, ivec2 *b)
{
  ivec2 tmp = *a;
  *a = *b;
  *b = tmp;
}

void draw_flat_bottom(int x0, int y0, int x1, int y1, ivec2 m, uint32_t color)
{
  int height = y1 - y0;
  float xs = x0;
  float xe = x0;
  float edge_left = (x1-x0)/(float)(height);
  float edge_right = (m.x-x0)/(float)(height);
  for(int i=0; i<height; i++)
  {
     draw_line( round(xs),y0+i, round(xe),y0+i,color);
     xs += edge_left;
     xe += edge_right;
  }

  color = 0xFFFF0000;
  draw_line(x0,y0,x1,y1,color);
  draw_line(x1,y1,m.x,m.y,color);
  draw_line(m.x,m.y,x0,y0,color);
}

void draw_flat_top(int x1, int y1, ivec2 m, int x2, int y2, uint32_t color)
{
  int height = y2 - y1;
  float xs = x1;
  float xe = m.x;
  float edge_left = (x2-x1)/(float)(height);
  float edge_right = (x2-m.x)/(float)(height);
  for(int i=0; i<height; i++)
  {
     draw_line( round(xs),y1+i, round(xe),y1+i,color);
     xs += edge_left;
     xe += edge_right;
  }

  color = 0xFFFF0000;
  draw_line(m.x,m.y,x1,y1,color);
  draw_line(x1,y1,x2,y2,color);
  draw_line(x2,y2,m.x,m.y,color);
}

ivec2 getMidpointPikuma(ivec2 p0, ivec2 p1, ivec2 p2)
{
  // Solution to the Triangle Midpoint
  ivec2 p0p1 = {p1.x - p0.x, p1.y - p0.y};
  ivec2 p0p2 = {p2.x - p0.x, p2.y - p0.y};
  float mx = p0.x + (p0p2.x*p0p1.y)/(float)p0p2.y;
  return (ivec2) {
    .x = (int) mx,
    .y = p1.y
  };
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    ivec2 sorted[3] = { {x0,y0}, {x1,y1}, {x2,y2} };
    if ( sorted[0].y > sorted[1].y ) ivec2_swap( &sorted[0], &sorted[1] );
    if ( sorted[1].y > sorted[2].y ) ivec2_swap( &sorted[1], &sorted[2] );
    if ( sorted[0].y > sorted[1].y ) ivec2_swap( &sorted[0], &sorted[1] );

    ivec2 midpoint = getMidpointPikuma(sorted[0], sorted[1], sorted[2]);
    draw_flat_bottom(sorted[0].x, sorted[0].y, sorted[1].x, sorted[1].y, midpoint, color);
    draw_flat_top(sorted[1].x, sorted[1].y, midpoint, sorted[2].x, sorted[2].y, color);


}
