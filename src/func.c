#include "func.h"

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
    /*
    if (width < 1 || height < 1) return;
    if (x >= window_width || y >= window_height) return;
    int startx = clamp(x, 0, window_width-1);
    int starty = clamp(y, 0, window_height-1);
    int endx = startx+width;
    int endy = starty+height;
    if (endx <= 0 || endy <= 0) return;
    endx = clamp(endx, 0, window_width-1);
    endy = clamp(endy, 0, window_height-1);
    */
    int startx=x;
    int starty=y;
    int endx=x+width;
    int endy=y+height;
    for(int cx=startx; cx<endx; cx++)
        for(int cy=starty; cy<endy; cy++)
        {
            setpix(cx,cy,color);
            //color_buffer[cy*window_width+cx] = color;
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

static int abs(int v)
{
    if (v >= 0) return v;
    return -v;
}

void line(int x0, int y0, int x1, int y1, uint32_t color)
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
