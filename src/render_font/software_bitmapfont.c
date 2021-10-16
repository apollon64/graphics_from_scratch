#ifndef SOFTWARE_BITMAPFONT_H
#define SOFTWARE_BITMAPFONT_H

#include "software_bitmapfont.h"

#include "font8x8.h"
#include "font8x14.h"
#include "font8x16.h"
#include <string.h>
#include <stdio.h> // vsnprintf
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <assert.h>

#include "../display.h"

static bool initialized = false;

enum eFONT_SIZE {
    FONT_8x8,
    FONT_8x14,
    FONT_8x16,
    FONT_INVALID
};

struct selected_font_properties
{
    int width, height, arraySize;
    int base; // Distance between lines. Font lingo.
    unsigned char* fontData;
};

struct global_draw_state{
    uint32_t color;
    float movex;
    float movey;
    enum eFONT_SIZE selected_font;
    struct selected_font_properties selected_font_properties;
} global_draw_state;

//Surface* surface;
char buffer[8*1024]; // Assert you never want to print more than 8k chars on screen.
// However, at 1920x1080 you could fit about 32k characters.
// 1920/8 = 240
// 1080/8 = 135
// 240*135 = 32400


void moveto(float x, float y)
{
    global_draw_state.movex = x;
    global_draw_state.movey = y;
}

void PrintCString( char* a_String, int x1, int y1)
{
    unsigned char* fontData = global_draw_state.selected_font_properties.fontData;
    int fontWidth = global_draw_state.selected_font_properties.width;
    int fontHeight = global_draw_state.selected_font_properties.height;
    int fontStepY = global_draw_state.selected_font_properties.base;
    int translateY = 0;

    int plotx = x1;
    int ploty = y1;

    for (int i = 0; i < (int)(strlen(a_String)); i++)
    {
        int pos = a_String[i];
        if (a_String[i]=='\n') {
            plotx = x1;
            global_draw_state.movey += fontHeight;
            ploty += fontStepY;
            translateY += fontStepY;
            continue;
        }
        for ( int y = 0; y < fontHeight; y++ )
        {
            int idx = fontHeight*pos + y;
            assert(idx < global_draw_state.selected_font_properties.arraySize);
            unsigned char c = fontData[idx];
            for (int x = 0; x < fontWidth; x++)
            {
                int bit = (c >> (fontWidth-x) ) & 0x1;
                int px = plotx+x;
                int py = ploty+y;

//                if (x<0) return;
//                if (x>=window_width) return;
//                if (y<0) return;
//                if (y>=window_height) return;
//                color_buffer[y*window_width+x] = color;

                if (bit && px < (int)(window_width) && py < (int)(window_height)) setpix(px,py,global_draw_state.color);
            }
        }
        plotx += fontWidth;
    }
    global_draw_state.movex = x1;
    global_draw_state.movey = y1 + translateY;
}

void gprintf(const char *fmt,  ...)
{
    if (!initialized) FontPainterInit();
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    PrintCString(buffer, global_draw_state.movex, global_draw_state.movey);
    va_end(ap);
}

void setfont(int width, int height)
{
    if (width==8 && height==8)
    {
        global_draw_state.selected_font = FONT_8x8;
    }
    else if (width==8 && height==14)
    {
        global_draw_state.selected_font = FONT_8x14;
    }
    else if (width==8 && height==16)
    {
        global_draw_state.selected_font = FONT_8x16;
    }
    struct selected_font_properties props;
    switch(global_draw_state.selected_font)
    {
#define ElementsOf(x) sizeof(x)/sizeof(x[0])
    case FONT_8x8:
        props.width = 8;
        props.height = 8;
        props.base = 9;
        props.fontData = font8x8;
        props.arraySize = ElementsOf(font8x8);
        break;
    case FONT_8x14:
        props.width = 8;
        props.height = 14;
        props.base = 15;
        props.fontData = font8x14;
        props.arraySize = ElementsOf(font8x14);
        break;
    case FONT_8x16:
        props.width = 8;
        props.height = 16;
        props.base = 17;
        props.fontData = font8x16;
        props.arraySize = ElementsOf(font8x16);
        break;
#undef ElementsOf
    case FONT_INVALID:
        assert(0);
    }
    global_draw_state.selected_font_properties = props;
}

FontPainterInit()
{
    setfont(8,8);
    global_draw_state.color = 0xFFFFFFFF;
    global_draw_state.movex = 0;
    global_draw_state.movey = 0;
    initialized = true;
}

#endif
