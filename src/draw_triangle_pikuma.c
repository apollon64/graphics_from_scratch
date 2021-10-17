
#include "display.h"
#include "triangle.h"
#include "draw_triangle_pikuma.h"


#include <assert.h>
#include <math.h>
#include "func.h"

static void draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < window_width && y >= 0 && y < window_height) {
        color_buffer[(window_width * y) + x] = color;
    }
}

static void draw_line_p(int x0, int y0, int x1, int y1, uint32_t color) {
    int delta_x = (x1 - x0);
    int delta_y = (y1 - y0);

    int longest_side_length = (abs(delta_x) >= abs(delta_y)) ? abs(delta_x) : abs(delta_y);

    float x_inc = delta_x / (float)longest_side_length;
    float y_inc = delta_y / (float)longest_side_length;

    float current_x = x0;
    float current_y = y0;

    for (int i = 0; i <= longest_side_length; i++) {
        draw_pixel(roundf(current_x), roundf(current_y), color);
        current_x += x_inc;
        current_y += y_inc;
    }
}


///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat bottom
/*
//
//        (x0,y0)
//          / \
//         /   \
//        /     \
//       /       \
//      /         \
//  (x1,y1)------(x2,y2)
//
*/
void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // Find the two slopes (two triangle legs)
    float inv_slope_1 = (float)(x1 - x0) / (y1 - y0);
    float inv_slope_2 = (float)(x2 - x0) / (y2 - y0);

    // Start x_start and x_end from the top vertex (x0,y0)
    float x_start = x0;
    float x_end = x0;

    // Loop all the scanlines from top to bottom
    for (int y = y0; y <= y2; y++) {
        draw_line_p(x_start, y, x_end, y, color);
        x_start += inv_slope_1;
        x_end += inv_slope_2;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat top
///////////////////////////////////////////////////////////////////////////////
//
//  (x0,y0)------(x1,y1)
//      \         /
//       \       /
//        \     /
//         \   /
//          \ /
//        (x2,y2)
//
///////////////////////////////////////////////////////////////////////////////
void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // Find the two slopes (two triangle legs)
    float inv_slope_1 = (float)(x2 - x0) / (y2 - y0);
    float inv_slope_2 = (float)(x2 - x1) / (y2 - y1);

    // Start x_start and x_end from the bottom vertex (x2,y2)
    float x_start = x2;
    float x_end = x2;

    // Loop all the scanlines from bottom to top
    for (int y = y2; y >= y0; y--) {
        draw_line_p(x_start, y, x_end, y, color);
        x_start -= inv_slope_1;
        x_end -= inv_slope_2;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled triangle with the flat-top/flat-bottom method
// We split the original triangle in two, half flat-bottom and half flat-top
/*
//
//          (x0,y0)
//            / \
//           /   \
//          /     \
//         /       \
//        /         \
//   (x1,y1)------(Mx,My)
//       \_           \
//          \_         \
//             \_       \
//                \_     \
//                   \    \
//                     \_  \
//                        \_\
//                           \
//                         (x2,y2)
//
*/
void draw_filled_triangle_p(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }

    if (y1 == y2) {
        // Draw flat-bottom triangle
        fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
    } else if (y0 == y1) {
        // Draw flat-top triangle
        fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
    } else {
        // Calculate the new vertex (Mx,My) using triangle similarity
        int My = y1;
        int Mx = (((x2 - x0) * (y1 - y0)) / (y2 - y0)) + x0;

        // Draw flat-bottom triangle
        fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);

        // Draw flat-top triangle
        fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Draw a triangle using three raw line calls
///////////////////////////////////////////////////////////////////////////////
void draw_triangle_p(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    draw_line_p(x0, y0, x1, y1, color);
    draw_line_p(x1, y1, x2, y2, color);
    draw_line_p(x2, y2, x0, y0, color);
}

///////////////////////////////////////////////////////////////////////////////
// Return the barycentric weights alpha, beta, and gamma for point p
/*
//
//          A
//         /|\
//        / | \
//       /  |  \
//      /  (p)  \
//     /  /   \  \
//    / /       \ \
//   B-------------C
//
*/
static vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
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
    float gamma = 1.0f - alpha - beta;

    vec3_t weights = { alpha, beta, gamma };
    return weights;
}

///////////////////////////////////////////////////////////////////////////////
// Function to draw the textured pixel at position x and y using interpolation
///////////////////////////////////////////////////////////////////////////////
void draw_texel(
    int x, int y, uint32_t* texture,
    vec4_t point_a, vec4_t point_b, vec4_t point_c,
    tex2_t a_uv, tex2_t b_uv, tex2_t c_uv
) {
    vec2_t p = { x, y };
    vec2_t a = vec2_from_vec4(point_a);
    vec2_t b = vec2_from_vec4(point_b);
    vec2_t c = vec2_from_vec4(point_c);

    vec3_t weights = barycentric_weights(a, b, c, p);

    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;
    const float EPS = 0.00001f;
    if ( alpha < -EPS || beta < -EPS || gamma < -EPS)
    {
        draw_pixel(x, y, 0xFF00FF32);// BGR
        //assert(alpha >= -EPS && "out side of alpha");
        //assert(beta >= -EPS && "out side of beta");
        //assert(gamma >= -EPS && "out side of gamma");
        //assert( (alpha+beta+gamma) <= 1.000001f && "bary out side of 1.");
        return;
    }
    if ( (alpha+beta+gamma) > 1.0f )
    {
        draw_pixel(x, y, 0x00550055);// R
        return;
    }
    if ( alpha != alpha)
    {
        draw_pixel(x, y, 0xFFFFFFFF);// white
        return;
    }
    if ( beta != beta)
    {
        draw_pixel(x, y, 0xFFFFFFFF);// BGR
        return;
    }
    if ( gamma != gamma)
    {
        draw_pixel(x, y, 0xFFFFFFFF);// BGR
        return;
    }

    if ( point_a.w < EPS)
    {
        draw_pixel(x, y, 0xFFFFFFFF);// BGR
        return;
    }

    // Variables to store the interpolated values of U, V, and also 1/w for the current pixel
    float interpolated_u;
    float interpolated_v;
    float interpolated_reciprocal_w;

    float persp_ax = a_uv.u / point_a.w;
    float persp_ay = a_uv.v / point_a.w;
    float persp_bx = b_uv.u / point_b.w;
    float persp_by = b_uv.v / point_b.w;
    float persp_cx = c_uv.u / point_c.w;
    float persp_cy = c_uv.v / point_c.w;

    // Perform the interpolation of all U/w and V/w values using barycentric weights and a factor of 1/w
    interpolated_u = persp_ax * alpha + persp_bx * beta + persp_cx * gamma;
    interpolated_v = persp_ay * alpha + persp_by * beta + persp_cy * gamma;

    // Also interpolate the value of 1/w for the current pixel
    interpolated_reciprocal_w = (1.0f / point_a.w) * alpha + (1.0f / point_b.w) * beta + (1.0f / point_c.w) * gamma;

    // Adjust 1/w so the pixels that are closer to the camera have smaller values
    float z = 1.0 - interpolated_reciprocal_w;

    // Only draw the pixel if the depth value is less than the one previously stored in the z-buffer
    if (z < z_buffer[(window_width * y) + x] && z > .01 && z < 1.) {

        // Update the z-buffer value with the 1/w of this current pixel
        z_buffer[(window_width * y) + x] = z;

        // Now we can divide back both interpolated values by 1/w
        interpolated_u /= interpolated_reciprocal_w;
        interpolated_v /= interpolated_reciprocal_w;

        // Map the UV coordinate to the full texture width and height
        int tex_x = abs((int)(interpolated_u * texture_width));
        int tex_y = abs((int)(interpolated_v * texture_height));
        //tex_x %= texture_width;
        //tex_y %= texture_height;
        bool oob = tex_x < 0 || tex_y < 0 || tex_x > texture_width-1 || tex_y > texture_width-1;
        uint32_t color = oob ? 0xFF550055 : texture[(texture_width * tex_y) + tex_x];
        /*U8 red = (U8)(alpha*255.f) & 0xFF;
        U8 green = (U8)(beta*255.f) & 0xFF;
        U8 blue = (U8)(gamma*255.f) & 0xFF;
        color = (255u << 24) | (red<<16) | (green<<8) | blue;*/
        draw_pixel(x, y, color);
    }


}

void draw_triangle_textured_p(vertex_texcoord_t p0, vertex_texcoord_t p1, vertex_texcoord_t p2, uint32_t *texture) //, uint32_t* colors, float area2)
{
    float fx0 = p0.x;
    float fy0 = p0.y;
    float z0 = p0.z;
    float w0 = p0.w;
    float u0 = p0.u;
    float v0 = p0.v;

    float fx1 = p1.x;
    float fy1 = p1.y;
    float z1 = p1.z;
    float w1 = p1.w;
    float u1 = p1.u;
    float v1 = p1.v;

    float fx2 = p2.x;
    float fy2 = p2.y;
    float w2 = p2.w;
    float z2 = p2.z;
    float u2 = p2.u;
    float v2 = p2.v;

    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    if (fy0 > fy1) {
        float_swap(&fy0, &fy1);
        float_swap(&fx0, &fx1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }
    if (fy1 > fy2) {
        float_swap(&fy1, &fy2);
        float_swap(&fx1, &fx2);
        float_swap(&z1, &z2);
        float_swap(&w1, &w2);
        float_swap(&u1, &u2);
        float_swap(&v1, &v2);
    }
    if (fy0 > fy1) {
        float_swap(&fy0, &fy1);
        float_swap(&fx0, &fx1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }

    // Flip the V component to account for inverted UV-coordinates (V grows downwards)
    v0 = 1.0f - v0;
    v1 = 1.0f - v1;
    v2 = 1.0f - v2;

    fx0 = fx0 - .5f;
    fy0 = fy0 - .5f;
    fx1 = fx1 - .5f;
    fy1 = fy1 - .5f;
    fx2 = fx2 - .5f;
    fy2 = fy2 - .5f;

    //int ix0 = (int)fx0;
    int iy0 = (int)fy0;
    //int ix1 = (int)fx1;
    int iy1 = (int)fy1;
    //int ix2 = (int)fx2;
    int iy2 = (int)fy2;

    // Create vector points and texture coords after we sort the vertices
    vec4_t point_a = { fx0, fy0, z0, w0 };
    vec4_t point_b = { fx1, fy1, z1, w1 };
    vec4_t point_c = { fx2, fy2, z2, w2 };
    tex2_t a_uv = { u0, v0 };
    tex2_t b_uv = { u1, v1 };
    tex2_t c_uv = { u2, v2 };

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

                // OPTIM remove this if real clipping
                if (x < 0 || x > window_width-1) continue;
                if (y < 0 || y > window_height-1) continue;

                // Draw our pixel with the color that comes from the texture
                draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
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

                if (x < 0 || x > window_width-1) continue;
                if (y < 0 || y > window_height-1) continue;

                // Draw our pixel with the color that comes from the texture
                draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
            }
        }
    }
}
