#include "draw_triangle_torb.h"
#include "triangle.h"

#include "func.h"
#include "misc.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static inline vec3_t barycentric_weights_from_coefficents(float x, float y, vec3_t A, vec3_t B, vec3_t C, float rarea2)
{
    float weight0 = x * A.x + y * A.y + A.z;
    float weight1 = x * B.x + y * B.y + B.z;
    float weight2 = x * C.x + y * C.y + C.z;

    // We can either re-use area calculation?
    // float rlen = rarea2;
    // or re-calculate
    float rlen = 1.0f/(weight0+weight1+weight2);
    weight0 *= rlen;
    weight1 *= rlen;
    //weight2 *= rlen;

    return (vec3_t) {
        // We can either calculate all 3
        //weight0, weight1, weight2
        // or infer last one
        weight0, weight1, 1.0f - (weight0 + weight1)
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

    // Weight alpha i`s the area of subtriangle BCP divided by the area of the full triangle ABC
    float alpha = (bc.x * bp.y - bp.x * bc.y) / area_triangle_abc;

    // Weight beta is the area of subtriangle ACP divided by the area of the full triangle ABC
    float beta = (ap.x * ac.y - ac.x * ap.y) / area_triangle_abc;

    // Weight gamma is easily found since barycentric cooordinates always add up to 1
    float gamma = 1 - alpha - beta;

    vec3_t weights = { alpha, beta, gamma };
    return weights;
}*/


static void interpolate_color(int x, int y, float area2,
                           vec3_t coeffA,
                           vec3_t coeffB,
                           vec3_t coeffC,
                           float w0, float w1, float w2,
                           uint32_t* colors)
{
    //vec3_t weights = barycentric_weights( (vec2_t){p0.x,p0.y}, (vec2_t){p1.x,p1.y}, (vec2_t){p2.x,p2.y}, (vec2_t){x+.5f,y+.5f});
    vec3_t weights = barycentric_weights_from_coefficents(x+.5f, y+.5f, coeffA, coeffB, coeffC, area2);

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
    //float one_over_one_over_w = 1.0f  / one_over_w;
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

    float* z_buffer = pk_z_buffer();
    float buffer_z = z_buffer[(pk_window_width() * y) + x];
    if ( interpolated_z < buffer_z ) {
        //draw_texel(x, y, u, v, texture);
        setpix(x,y,colors[0]);
        z_buffer[(pk_window_width() * y) + x] = interpolated_z;
    }
    /*uint32_t color = 0xFF000000;
    U8 red =   (U8)clampf( 255*rweights.x,0,255) & 0xFF;
    U8 green = (U8)clampf( 255*rweights.y,0,255) & 0xFF;
    U8 blue =  (U8)clampf( 255*rweights.z,0,255) & 0xFF;
    color = (255u << 24) | (red<<16) | (green<<8) | blue;
    setpix(x,y,color);*/
}

static /*inline*/ void draw_texel(int x, int y, float u, float v, texture_t* texture)
{
    // Map the UV coordinate to the full texture width and height
    //int tex_x = abs((int)(u * texture->width));
    //int tex_y = abs((int)(v * texture->height));
    unsigned tex_x = u * texture->width;
    unsigned tex_y = v * texture->height;

    // Allow for texture wrap, either by subtracting
    tex_x = tex_x > texture->width ? tex_x - texture->width : tex_x;
    tex_y = tex_y > texture->height ? tex_y - texture->height : tex_y;
    // or by modulo
    //tex_x %= texture->width;
    //tex_y %= texture->height;
    unsigned tex_idx = tex_y * texture->width + tex_x;
    //assert(tex_idx >= 0 && "tex idx less 0");
    //assert(tex_idx <= texture->width*texture->height*4 && "tex idx oob");
    uint32_t texel = texture->texels[tex_idx];

    //uint32_t color = 0xFFFFFFFF;
    //uint32_t texel_lit = mix_colors( packColor(tex_r, tex_g, tex_b), color, .5f);
    //setpix(x,y, texel_lit);
    setpix_no_bound_check(x, y, texel);
    //setpix(x,y, packColor(u*255, v*255, (1-u-v)*255 ) );
}

static void interpolate_uv(int x, int y, float z, float area2,
                           vec3_t coeffA,
                           vec3_t coeffB,
                           vec3_t coeffC,
                           vertex_texcoord_t p0, vertex_texcoord_t p1, vertex_texcoord_t p2,
                           texture_t* texture)
{
    float* z_buffer = pk_z_buffer();
    float buffer_z = z_buffer[(pk_window_width() * y) + x];
    if ( z < buffer_z ) {
        vec3_t weights = barycentric_weights_from_coefficents(x+.5f, y+.5f, coeffA, coeffB, coeffC, area2);
        // Also interpolate the value of 1/w for the current pixel
        float one_over_w = p0.w * weights.x + p1.w * weights.y + p2.w * weights.z;

//        if (0)
//        {
//        U8 red =   (U8)clampf( weights.x * 255, 0.0, 255.0);
//        U8 green = (U8)clampf( weights.y * 255 ,0.0, 255.0);
//        U8 blue =  (U8)clampf( weights.z * 255 ,0.0, 255.0);
//        uint32_t color = packColor(red,green,blue);
//        setpix(x,y,color);
//        return;
//        }

//        // Z can be found in multiple ways. From W. From barycentric lerp. From a plane equation at x*a + y*b + c. Or from creating gradients for scanline to step.
//        float interpolated_z;
//        if(0)
//        {
//            //interpolated_z = p0.z * weights.x + p1.z * weights.y + p2.z * weights.z;
//            //interpolated_z *= one_over_one_over_w;

//            // Subtract 1 from W since W goes from 1.0 to 0.0, and we want 0-1
//            interpolated_z = 1.0f - interpolated_z;
//        } else {
//            interpolated_z = 1.0f - one_over_w;
//            //float interpolated_z = 1.0f - (one_over_w*rlen);
//        }

        const float EPS = 0.0001f;
        if ( weights.x < -EPS || weights.y < -EPS || weights.z < -EPS)
        {
            setpix(x, y, 0xFF00FF32);// BGR
            //if(normalized_weights.x < -EPS) printf("out side of a by %f.  %f\n", normalized_weights.x, weights.x);
            //if(normalized_weights.y < -EPS) printf("out side of b by %f.  %f\n", normalized_weights.y, weights.y);
            //if(normalized_weights.z < -EPS) printf("out side of c by %f.  %f\n", normalized_weights.z, weights.z);
            float sum = weights.x + weights.y + weights.z;
            if( sum >= (1+EPS) ) setpix(x, y, packColor(255,255,0) );
            if( sum <= (0-EPS) ) setpix(x, y, packColor(0,255,255) );
            return;
        }

        // Doing UV interpolation only if we pass Z test helps a lot!
        // We could avoid Z interpolation also if we saved Z as a plane or did interpolation in rasterizer/scanline
        float u = p0.u * weights.x + p1.u * weights.y + p2.u * weights.z;
        float v = p0.v * weights.x + p1.v * weights.y + p2.v * weights.z;

        float one_over_one_over_w = 1.0f  / one_over_w;
        u *= one_over_one_over_w;
        v *= one_over_one_over_w;

        draw_texel(x, y, u, v, texture);
        z_buffer[(pk_window_width() * y) + x] = z;
    }

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
    // Should re-use
}

float getDepth(depthplane_t depthplane, float sample_x, float sample_y) {
  //  Interpolated result:
  return sample_x * depthplane.a + sample_y * depthplane.b + depthplane.c;
}

 depthplane_t initDepthPlane(vec4_t v[3]) {

   // Delta between coordinates
   double xd[3];
   double yd[3];
   for (int i = 0; i < 3; i++)
   {
       xd[i] = v[i].x - v[(i + 1) % 3].x;
       yd[i] = v[i].y - v[(i + 1) % 3].y;
   }

   double x0_t = 0;
   double y0_t = 0;
   double xt[3];
   double yt[3];
   for (int i = 0; i < 3; i++)
   {
       xt[i] = v[i].x - x0_t;
       yt[i] = v[i].y - y0_t;
   }

   // Z slope / delta
   double z10 = v[1].z - v[0].z;
   double z20 = v[2].z - v[0].z;

   // 2x area
   double area2 = xd[0] * -yd[2] + xd[2] * yd[0];

   // Reciprocal of 2x area
   double divisor = 1.0 / area2;

   // Un-normalized Depth Plane
   double apz = z10 * yd[2] + z20 * yd[0];
   double bpz = (-z10) * xd[2] + (-z20) * xd[0];
   double cpz = (-apz) * xt[0] + (-bpz) * yt[0];

   // Normalized depth plane
   depthplane_t depthplane;
   depthplane.a = apz * divisor;
   depthplane.b = bpz * divisor;
   depthplane.c = cpz * divisor + v[0].z;
   depthplane.area2 = divisor;
   return depthplane;
}

void draw_triangle_textured(vertex_texcoord_t p0, vertex_texcoord_t p1, vertex_texcoord_t p2,
                            texture_t *texture, uint32_t* colors, float area2)
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

    vec4_t verts[3];
    verts[0] =(vec4_t) {p0.x, p0.y, p0.z, 0.0f};
    verts[1] =(vec4_t) {p1.x, p1.y, p1.z, 0.0f};
    verts[2] =(vec4_t) {p2.x, p2.y, p2.z, 0.0f};
    depthplane_t dplane = initDepthPlane(verts);

    // go back and up half a pixel
    float fx0 = p0.x - .5f;
    float fy0 = p0.y - .5f;
    float fx1 = p1.x - .5f;
    float fy1 = p1.y - .5f;
    float fx2 = p2.x - .5f;
    float fy2 = p2.y - .5f;

    ivec2_t clipMin = {0,0};
    ivec2_t clipMax = {pk_window_width(), pk_window_height()};


    int iy0 = ceilf(fy0);
    int iy1 = ceilf(fy1);
    int iy2 = ceilf(fy2);
    iy0 = MIN(iy0, clipMax.y);
    iy1 = MIN(iy1, clipMax.y);
    iy2 = MIN(iy2, clipMax.y);
    iy0 = MAX(iy0, clipMin.y);
    iy1 = MAX(iy1, clipMin.y);
    iy2 = MAX(iy2, clipMin.y);

    if ( (iy0-iy2) == 0 ) {
        // No height. return early
        return;
    }
//    vec2_t a = vec2_sub( (vec2_t) {
//        p1.x,p1.y
//    }, (vec2_t) {
//        p2.x,p2.y
//    } );
//    vec2_t b = vec2_sub( (vec2_t) {
//        p0.x,p0.y
//    }, (vec2_t) {
//        p2.x,p2.y
//    } );
//    float area_triangle_abc = .5f * fabsf((a.x * b.y) - (a.y * b.x));

//    if ( area_triangle_abc <= 1.0f / 256.f)
//    {
//        //printf("tiny area. %f, area2/2=%f\n", area_triangle_abc, .5f * area2);
//        return;
//    }

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
    if (scan_height) {
        inv_slope_1 = (fx1 - fx0) / (fy1 - fy0);
        inv_slope_2 = (fx2 - fx0) / (fy2 - fy0);
        for (int y = iy0; y < iy1; y++) {
            int x_start = ceilf(fx1 + (y - fy1) * inv_slope_1);
            int x_end = ceilf(fx0 + (y - fy0) * inv_slope_2);
            x_start = MIN(x_start, clipMax.x);
            x_end   = MIN(x_end, clipMax.x);
            x_start = MAX(x_start, clipMin.x);
            x_end   = MAX(x_end, clipMin.x);

            if (x_end < x_start) {
                int_swap(&x_start, &x_end); // swap if x_start is to the right of x_end
            }


            for (int x = x_start; x < x_end; x++) {
                assert(x>=0);
                assert(x < pk_window_width() );
                assert(y>=0);
                assert(y < pk_window_height() );
                // Draw our pixel with the color that comes from the texture
                float z = getDepth(dplane,x,y);
                interpolate_uv( x, y, z, dplane.area2, coeffA, coeffB, coeffC, p0, p1, p2, texture);
            }
        }
    }

    ///////////////////////////////////////////////////////
    // Render the bottom part of the triangle (flat-top)
    ///////////////////////////////////////////////////////
    inv_slope_1 = 0;
    inv_slope_2 = 0;

    scan_height = iy2 - iy1;

    if (scan_height) {
        inv_slope_1 = (fx2 - fx1) / (fy2 - fy1);
        inv_slope_2 = (fx2 - fx0) / (fy2 - fy0);
        for (int y = iy1; y < iy2; y++) {
            int x_start = ceilf(fx1 + (y - fy1) * inv_slope_1);
            int x_end = ceilf(fx0 + (y - fy0) * inv_slope_2);
            x_start = MIN(x_start, clipMax.x);
            x_end   = MIN(x_end, clipMax.x);
            x_start = MAX(x_start, clipMin.x);
            x_end   = MAX(x_end, clipMin.x);
            if (x_end < x_start) {
                int_swap(&x_start, &x_end); // swap if x_start is to the right of x_end
            }

            for (int x = x_start; x < x_end; x++) {
                assert(x>=0);
                assert(x < pk_window_width() );
                assert(y>=0);
                assert(y < pk_window_height() );

                // Draw our pixel with the color that comes from the texture
                float z = getDepth(dplane,x,y);
                interpolate_uv( x, y, z, dplane.area2, coeffA, coeffB, coeffC, p0, p1, p2, texture);
            }
        }
    }
}
