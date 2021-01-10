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
    uint32_t ret = 0xFF000000;
    //U8 a = 0;
    //ret |= a << 24;
    ret |= b << 16;
    ret |= g << 8;
    ret |= r << 0;
    return ret;
}

void unpackColor(uint32_t c, float *r, float *g, float *b)
{
    *b = ( (c >> 16) & 0xFF) / 255.f;
    *g = ( (c >>  8) & 0xFF) / 255.f;
    *r = ( (c >>  0) & 0xFF) / 255.f;
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

static inline void setpix_no_bound_check(int x, int y, uint32_t color)
{
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

void draw_line3d(int x0, int y0, float w0, int x1, int y1, float w1, uint32_t color)
{
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
  float cw = 1.f/w0;
  for(int i=0; i<=side_len; i++) {
      int x = roundf(cx);
      int y = roundf(cy);
      //float one_over_w = lerp(1.f/z0, 1.f/z1, i/(float)side_len );
      float one_over_w = cw;
      float interpolated_z = 1.0f - one_over_w;
      if (interpolated_z < z_buffer[y*window_height+x])
      {
         setpix( x, y, color);
         z_buffer[y*window_height+x] = interpolated_z;
      }
      cx += x_inc;
      cy += y_inc;
      cw += w_inc;
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

static inline void draw_texel(int x, int y, float u, float v, texture_t* texture)
{
    // Map the UV coordinate to the full texture width and height
    int tex_x = abs((int)(u * texture->width));
    int tex_y = abs((int)(v * texture->height));

    tex_x %= texture->width;
    tex_y %= texture->height;
    int tex_idx = tex_y * texture->width + tex_x;
    assert(tex_idx >= 0 && "tex idx less 0");
    assert(tex_idx <= texture->width*texture->height*4 && "tex idx oob");

    U8 tex_r = texture->texels[ 4*(tex_idx)+0];
    U8 tex_g = texture->texels[ 4*(tex_idx)+1];
    U8 tex_b = texture->texels[ 4*(tex_idx)+2];

    //uint32_t color = 0xFFFFFFFF;
    //uint32_t texel_lit = mix_colors( packColor(tex_r, tex_g, tex_b), color, .5f);
    //setpix(x,y, texel_lit);
    setpix_no_bound_check(x,y, packColor(tex_r, tex_g, tex_b) );
    //setpix(x,y, packColor(u*255, v*255, (1-u-v)*255 ) );
}


static inline vec3_t barycentric_weights_from_coefficents(float x, float y, vec3_t A, vec3_t B, vec3_t C)
{
    float weight0 = x * A.x + y * A.y + A.z;
    float weight1 = x * B.x + y * B.y + B.z;
    float weight2 = x * C.x + y * C.y + C.z;
    return (vec3_t) {
        weight0, weight1, weight2
    };
}

/*static vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
    // Find the vectors between the vertices ABC and point p
    vec2_t ab = vec2_sub(b, a);
    vec2_t bc = vec2_sub(c, b);
    vec2_t ac = vec2_sub(c, a);
    vec2_t ap = vec2_sub(p, a);
    vec2_t bp = vec2_sub(p, b);

    // Calcualte the area of the full triangle ABC using cross product (area of parallelogram)
    float area_triangle_abc = (ab.x * ac.y - ab.y * ac.x);

    // Weight alpha is the area of subtriangle BCP divided by the area of the full triangle ABC
    float alpha = (bc.x * bp.y - bp.x * bc.y) / area_triangle_abc;

    // Weight beta is the area of subtriangle ACP divided by the area of the full triangle ABC
    float beta = (ap.x * ac.y - ac.x * ap.y) / area_triangle_abc;

    // Weight gamma is easily found since barycentric cooordinates always add up to 1
    float gamma = 1 - alpha - beta;

    vec3_t weights = { alpha, beta, gamma };
    return weights;
}*/

static void interpolate_color(int x, int y,
                           vec3_t coeffA,
                           vec3_t coeffB,
                           vec3_t coeffC,
                           float w0, float w1, float w2,
                           uint32_t* colors)
{
    //vec3_t weights = barycentric_weights( (vec2_t){p0.x,p0.y}, (vec2_t){p1.x,p1.y}, (vec2_t){p2.x,p2.y}, (vec2_t){x+.5f,y+.5f});
    vec3_t weights = barycentric_weights_from_coefficents(x+.5f, y+.5f, coeffA, coeffB, coeffC);

    vec3_t normalized_weights;
    // This is connected with the triangle area and W somehow...
    float rlen = 1.0f/(weights.x+weights.y+weights.z);
    normalized_weights.x = weights.x * rlen;
    normalized_weights.y = weights.y * rlen;
    normalized_weights.z = weights.z * rlen;
    weights = normalized_weights;

    //float u = p0.u * weights.x + p1.u * weights.y + p2.u * weights.z;
    //float v = p0.v * weights.x + p1.v * weights.y + p2.v * weights.z;
    // Also interpolate the value of 1/w for the current pixel
    float one_over_w = w0 * weights.x + w1 * weights.y + w2 * weights.z;
    float one_over_one_over_w = 1.0f  / one_over_w;
    //u *= one_over_one_over_w;
    //v *= one_over_one_over_w;

    //float interpolated_z = p0.z * weights.x + p1.z * weights.y + p2.z * weights.z;
    //interpolated_z *= one_over_one_over_w;

    // Subtract 1 from W since W goes from 1.0 to 0.0, and we want 0-1
    //float interpolated_z = 1.0f - (one_over_w*rlen);
    float interpolated_z = 1.0f - one_over_w;

    const float EPS = 0.0001f;
    if ( normalized_weights.x < -EPS || normalized_weights.y < -EPS || normalized_weights.z < -EPS)
    {
        setpix(x, y, 0xFF00FF32);// BGR
        //if(normalized_weights.x < -EPS) printf("out side of a by %f.  %f\n", normalized_weights.x, weights.x);
        //if(normalized_weights.y < -EPS) printf("out side of b by %f.  %f\n", normalized_weights.y, weights.y);
        //if(normalized_weights.z < -EPS) printf("out side of c by %f.  %f\n", normalized_weights.z, weights.z);
        float sum = normalized_weights.x + normalized_weights.y + normalized_weights.z;
        if( sum >= (1+EPS) ) setpix(x, y, packColor(255,255,0) );
        if( sum <= (0-EPS) ) setpix(x, y, packColor(0,255,255) );
        return;
    }

    float buffer_z = z_buffer[(window_width * y) + x];
    if ( interpolated_z < buffer_z ) {
        //draw_texel(x, y, u, v, texture);
        setpix(x,y,colors[0]);
        z_buffer[(window_width * y) + x] = interpolated_z;
    }
    /*uint32_t color = 0xFF000000;
    U8 red =   (U8)clampf( 255*rweights.x,0,255) & 0xFF;
    U8 green = (U8)clampf( 255*rweights.y,0,255) & 0xFF;
    U8 blue =  (U8)clampf( 255*rweights.z,0,255) & 0xFF;
    color = (255u << 24) | (red<<16) | (green<<8) | blue;
    setpix(x,y,color);*/
}

static void interpolate_uv(int x, int y,
                           vec3_t coeffA,
                           vec3_t coeffB,
                           vec3_t coeffC,
                           vertex_texcoord_t p0, vertex_texcoord_t p1, vertex_texcoord_t p2,
                           texture_t* texture)
{
    //vec3_t weights = barycentric_weights( (vec2_t){p0.x,p0.y}, (vec2_t){p1.x,p1.y}, (vec2_t){p2.x,p2.y}, (vec2_t){x+.5f,y+.5f});
    vec3_t weights = barycentric_weights_from_coefficents(x+.5f, y+.5f, coeffA, coeffB, coeffC);

    vec3_t normalized_weights;
    // This is connected with the triangle area and W somehow...
    float rlen = 1.0f/(weights.x+weights.y+weights.z);
    normalized_weights.x = weights.x * rlen;
    normalized_weights.y = weights.y * rlen;
    normalized_weights.z = weights.z * rlen;
    weights = normalized_weights;

    float u = p0.u * weights.x + p1.u * weights.y + p2.u * weights.z;
    float v = p0.v * weights.x + p1.v * weights.y + p2.v * weights.z;
    // Also interpolate the value of 1/w for the current pixel
    float one_over_w = p0.w * weights.x + p1.w * weights.y + p2.w * weights.z;
    float one_over_one_over_w = 1.0f  / one_over_w;
    u *= one_over_one_over_w;
    v *= one_over_one_over_w;

    //float interpolated_z = p0.z * weights.x + p1.z * weights.y + p2.z * weights.z;
    //interpolated_z *= one_over_one_over_w;

    // Subtract 1 from W since W goes from 1.0 to 0.0, and we want 0-1
    //float interpolated_z = 1.0f - (one_over_w*rlen);
    float interpolated_z = 1.0f - one_over_w;

    const float EPS = 0.0001f;
    if ( normalized_weights.x < -EPS || normalized_weights.y < -EPS || normalized_weights.z < -EPS)
    {
        setpix(x, y, 0xFF00FF32);// BGR
        //if(normalized_weights.x < -EPS) printf("out side of a by %f.  %f\n", normalized_weights.x, weights.x);
        //if(normalized_weights.y < -EPS) printf("out side of b by %f.  %f\n", normalized_weights.y, weights.y);
        //if(normalized_weights.z < -EPS) printf("out side of c by %f.  %f\n", normalized_weights.z, weights.z);
        float sum = normalized_weights.x + normalized_weights.y + normalized_weights.z;
        if( sum >= (1+EPS) ) setpix(x, y, packColor(255,255,0) );
        if( sum <= (0-EPS) ) setpix(x, y, packColor(0,255,255) );
        return;
    }

    float buffer_z = z_buffer[(window_width * y) + x];
    if ( interpolated_z < buffer_z ) {
        draw_texel(x, y, u, v, texture);
        z_buffer[(window_width * y) + x] = interpolated_z;
    }
    /*uint32_t color = 0xFF000000;
    U8 red =   (U8)clampf( 255*rweights.x,0,255) & 0xFF;
    U8 green = (U8)clampf( 255*rweights.y,0,255) & 0xFF;
    U8 blue =  (U8)clampf( 255*rweights.z,0,255) & 0xFF;
    color = (255u << 24) | (red<<16) | (green<<8) | blue;
    setpix(x,y,color);*/
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

static inline vec3_t makeEdge(float x0,float y0, float x1, float y1)
{
    float refx = 0.0;
    float refy = 0.0;
    float dx = x1 - x0;
    float dy = y1 - y0;
    float a = -dy;
    float b = dx;
    float c = -a * (x0-refx) - b*(y0-refy);
    return (vec3_t) {
        a, b, c
    };
}

void draw_triangle(
  float x0, float y0, float z0, float w0,
  float x1, float y1, float z1, float w1,
  float x2, float y2, float z2, float w2,
  uint32_t* colors)
{
    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    if ( y0 > y1 ) {
        float_swap( &x0, &x1 );
        float_swap( &y0, &y1 );
        float_swap( &z0, &z1 );
        float_swap( &w0, &w1 );
        uint32_t_swap( &colors[0], &colors[1] );
    }
    if ( y1 > y2 ) {
        float_swap( &x1, &x2 );
        float_swap( &y1, &y2 );
        float_swap( &z1, &z2 );
        float_swap( &w1, &w2 );
        uint32_t_swap( &colors[1], &colors[2] );
    }
    if ( y0 > y1 ) {
        float_swap( &x0, &x1 );
        float_swap( &y0, &y1 );
        float_swap( &z0, &z1 );
        float_swap( &w0, &w1 );
        uint32_t_swap( &colors[0], &colors[1] );
    }

    // go back and up half a pixel
    float fx0 = x0 - .5f;
    float fy0 = y0 - .5f;
    float fx1 = x1 - .5f;
    float fy1 = y1 - .5f;
    float fx2 = x2 - .5f;
    float fy2 = y2 - .5f;

    int iy0 = (int)fy0;
    int iy1 = (int)fy1;
    int iy2 = (int)fy2;
    if ( (iy0-iy2) == 0 ) {
        // No height. return early
        return;
    }
    vec2_t a = vec2_sub( (vec2_t) {
        x1,y1
    }, (vec2_t) {
        x2,y2
    } );
    vec2_t b = vec2_sub( (vec2_t) {
        x0,y0
    }, (vec2_t) {
        x2,y2
    } );
    float area2 = fabsf((a.x * b.y) - (a.y * b.x));
    float area_triangle_abc = .5f * fabsf((a.x * b.y) - (a.y * b.x));

    if ( area_triangle_abc <= 1.0f / 256.f)
    {
        //printf("tiny area. %f, area2/2=%f\n", area_triangle_abc, .5f * area2);
        return;
    }

    // make .w the reciprocal of w
    w0 = 1.0f / w0;
    w1 = 1.0f / w1;
    w2 = 1.0f / w2;

    vec3_t e0 = makeEdge( x0,y0, x1,y1 );
    vec3_t e1 = makeEdge( x1,y1, x2,y2 );
    vec3_t e2 = makeEdge( x2,y2, x0,y0 );
    // Coeffs equal edges, but rotated
    // Normalize coeffs by dividing by area squared. This gives us perspective.
    vec3_t coeffA = vec3_mul(e1, 1.f/area2);
    vec3_t coeffB = vec3_mul(e2, 1.f/area2);
    vec3_t coeffC = vec3_mul(e0, 1.f/area2);

    ///////////////////////////////////////////////////////
    // Render the upper part of the triangle (flat-bottom)
    ///////////////////////////////////////////////////////
    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    int scan_height = iy1 - iy0;
    if (scan_height) inv_slope_1 = (float)(fx1 - fx0) / fabsf(fy1 - fy0);
    if (scan_height) inv_slope_2 = (float)(fx2 - fx0) / fabsf(fy2 - fy0);

    if (scan_height) {
        for (int y = ceilf(fy0); y < ceilf(fy1); y++) {
            int x_start = ceilf(fx1 + (y - fy1) * inv_slope_1);
            int x_end = ceilf(fx0 + (y - fy0) * inv_slope_2);

            if (x_end < x_start) {
                int_swap(&x_start, &x_end); // swap if x_start is to the right of x_end
            }

            for (int x = x_start; x < x_end; x++) {
                interpolate_color( x, y, coeffA, coeffB, coeffC, w0, w1, w2, colors);
            }
        }
    }

    ///////////////////////////////////////////////////////
    // Render the bottom part of the triangle (flat-top)
    ///////////////////////////////////////////////////////
    inv_slope_1 = 0;
    inv_slope_2 = 0;

    scan_height = iy2 - iy1;
    if (scan_height) inv_slope_1 = (float)(fx2 - fx1) / fabsf(fy2 - fy1);
    if (scan_height) inv_slope_2 = (float)(fx2 - fx0) / fabsf(fy2 - fy0);

    if (scan_height) {
        for (int y = ceilf(fy1); y < ceilf(fy2); y++) {
            int x_start = ceilf(fx1 + (y - fy1) * inv_slope_1);
            int x_end = ceilf(fx0 + (y - fy0) * inv_slope_2);

            if (x_end < x_start) {
                int_swap(&x_start, &x_end); // swap if x_start is to the right of x_end
            }

            for (int x = x_start; x < x_end; x++) {
                interpolate_color( x, y, coeffA, coeffB, coeffC, w0, w1, w2, colors);
            }
        }
    }
}


void draw_triangle_textured(vertex_texcoord_t p0, vertex_texcoord_t p1, vertex_texcoord_t p2, texture_t *texture, uint32_t* colors, float area2)
{
    if ( isnan(p0.x) ) {
        printf("x0 is nan\n");
        return;
    }
    if ( isnan(p0.y) ) {
        printf("y0 is nan\n");
        return;
    }
    if ( isnan(p1.x) ) {
        printf("x1 is nan\n");
        return;
    }
    if ( isnan(p1.y) ) {
        printf("y1 is nan\n");
        return;
    }
    if ( isnan(p2.x) ) {
        printf("x2 is nan\n");
        return;
    }
    if ( isnan(p2.y) ) {
        printf("y2 is nan\n");
        return;
    }

    if ( isnan(p0.w) ) {
        printf("w0 is nan\n");
        return;
    }
    if ( isnan(p1.w) ) {
        printf("w1 is nan\n");
        return;
    }
    if ( isnan(p2.w) ) {
        printf("w2 is nan\n");
        return;
    }

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

    // go back and up half a pixel
    float fx0 = p0.x - .5f;
    float fy0 = p0.y - .5f;
    float fx1 = p1.x - .5f;
    float fy1 = p1.y - .5f;
    float fx2 = p2.x - .5f;
    float fy2 = p2.y - .5f;

    //int ix0 = (int)fx0;
    int iy0 = (int)fy0;
    //int ix1 = (int)fx1;
    int iy1 = (int)fy1;
    //int ix2 = (int)fx2;
    int iy2 = (int)fy2;
    if ( (iy0-iy2) == 0 ) {
        // No height. return early
        return;
    }
    vec2_t a = vec2_sub( (vec2_t) {
        p1.x,p1.y
    }, (vec2_t) {
        p2.x,p2.y
    } );
    vec2_t b = vec2_sub( (vec2_t) {
        p0.x,p0.y
    }, (vec2_t) {
        p2.x,p2.y
    } );
    float area_triangle_abc = .5f * fabsf((a.x * b.y) - (a.y * b.x));

    if ( area_triangle_abc <= 1.0f / 256.f)
    {
        //printf("tiny area. %f, area2/2=%f\n", area_triangle_abc, .5f * area2);
        return;
    }

    p0.v = 1.0f - p0.v;
    p1.v = 1.0f - p1.v;
    p2.v = 1.0f - p2.v;

    (void)&colors[0];
    //uint32_t first_color = colors[0];

// pre-divide u,v by w
    p0.u /= p0.w;
    p1.u /= p1.w;
    p2.u /= p2.w;

    p0.v /= p0.w;
    p1.v /= p1.w;
    p2.v /= p2.w;

// pre-divide z by w
    /*p0.z /= p0.w;
    p1.z /= p1.w;
    p2.z /= p2.w;*/

// make .w the reciprocal of w
    p0.w = 1.0f / p0.w;
    p1.w = 1.0f / p1.w;
    p2.w = 1.0f / p2.w;

    vec3_t e0 = makeEdge( p0.x,p0.y, p1.x,p1.y );
    vec3_t e1 = makeEdge( p1.x,p1.y, p2.x,p2.y );
    vec3_t e2 = makeEdge( p2.x,p2.y, p0.x,p0.y );
    // Coeffs equal edges, but rotated
    // Normalize coeffs by dividing by area squared. This gives us perspective.
    vec3_t coeffA = vec3_mul(e1, 1.f/area2);
    vec3_t coeffB = vec3_mul(e2, 1.f/area2);
    vec3_t coeffC = vec3_mul(e0, 1.f/area2);

    ///////////////////////////////////////////////////////
    // Render the upper part of the triangle (flat-bottom)
    ///////////////////////////////////////////////////////
    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    int scan_height = iy1 - iy0;
    if (scan_height) inv_slope_1 = (float)(fx1 - fx0) / fabsf(fy1 - fy0);
    if (scan_height) inv_slope_2 = (float)(fx2 - fx0) / fabsf(fy2 - fy0);

    if (scan_height) {
        for (int y = ceilf(fy0); y < ceilf(fy1); y++) {
            int x_start = ceilf(fx1 + (y - fy1) * inv_slope_1);
            int x_end = ceilf(fx0 + (y - fy0) * inv_slope_2);

            if (x_end < x_start) {
                int_swap(&x_start, &x_end); // swap if x_start is to the right of x_end
            }

            for (int x = x_start; x < x_end; x++) {
                // Draw our pixel with the color that comes from the texture
                //setpix(x,y,0xFF0000FF);
                //draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
                interpolate_uv( x, y, coeffA, coeffB, coeffC, p0, p1, p2, texture);
            }
        }
    }

    ///////////////////////////////////////////////////////
    // Render the bottom part of the triangle (flat-top)
    ///////////////////////////////////////////////////////
    inv_slope_1 = 0;
    inv_slope_2 = 0;

    scan_height = iy2 - iy1;
    if (scan_height) inv_slope_1 = (float)(fx2 - fx1) / fabsf(fy2 - fy1);
    if (scan_height) inv_slope_2 = (float)(fx2 - fx0) / fabsf(fy2 - fy0);

    if (scan_height) {
        for (int y = ceilf(fy1); y < ceilf(fy2); y++) {
            int x_start = ceilf(fx1 + (y - fy1) * inv_slope_1);
            int x_end = ceilf(fx0 + (y - fy0) * inv_slope_2);

            if (x_end < x_start) {
                int_swap(&x_start, &x_end); // swap if x_start is to the right of x_end
            }

            for (int x = x_start; x < x_end; x++) {
                // Draw our pixel with the color that comes from the texture
                //draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
                interpolate_uv( x, y, coeffA, coeffB, coeffC, p0, p1, p2, texture);
            }
        }
    }
}
