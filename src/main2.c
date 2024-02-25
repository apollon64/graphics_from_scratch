#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include "func.h"
#include "texture.h"
#include "draw_triangle_pikuma.h"
#include "draw_triangle_torb.h"
#include "render_font/software_bitmapfont.h"

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void iterateDrawcalls(texture_t texture) {
    clear_color_buffer( packColorRGB(255,0,255) );
    clear_z_buffer( 1.0f );
    draw_grid();

    draw_triangle_textured_p(
    (vertex_texcoord_t){ 0,   pk_window_height()-0, 0.0f,   1.0f, 0.0f, 0.0f },
    (vertex_texcoord_t){ 100, pk_window_height()-100, 0.0f, 1.0f, 1.0f, 1.0f},
    (vertex_texcoord_t){ 100, pk_window_height()-00, 0.0f,  1.0f, 1.0f, 0.0f},
    texture
    );

    // A B G R
    uint32_t red = packColorRGB(255,0,0);
    uint32_t green = packColorRGB(0,255,0);
    uint32_t blue = packColorRGB(0,0,255);
    uint32_t yellow = packColorRGB(255,255,0);
    draw_filled_triangle_p( 0,   0, 100, 100, 0 , 100, red);
    draw_filled_triangle_p( 100, 0, 200, 100, 100 , 100, green);
    draw_filled_triangle_p( 200, 0, 300, 100, 200 , 100, blue);
    draw_filled_triangle_p( 300, 0, 400, 100, 300 , 100, yellow);

    for (int i=0; i<4; i++)
        circle(100 + i * 50, 200, 20);

    draw_line(0,0,pk_window_width(), pk_window_height(), packColorRGB(0,0,0) );

    moveto(0,0);
    gprintf("Hello world\n");
}


int main(int argc, char *argv[])
{
    printf("Hello Triangle World %s\n", __DATE__);

    // Allocate a color and depth buffer
    int xres = 400;
    int yres = 240;

    // Color is packed into 32-bits (32/8 = 4byte) which is enough for 8 bits per channel
    uint32_t* color_buffer = (uint32_t*)malloc(xres*yres*sizeof(uint32_t));

    // Depth is simply a rectangle of floats
    float* z_buffer = (float*)malloc(xres*yres*sizeof(float));

    // Set these allocated pointers to the PikumaSoft library
    pk_init( color_buffer, z_buffer, xres, yres );

    // Load some assets
    //const char* mesh_file = "./assets/cube.obj";
    const char* texture_file = "./assets/cube.png";

    //load_obj_file_data(mesh_file);
    texture_t texture = load_png_texture_data(texture_file);

    // Actually render out scene to the color and zbuffer! Exciting!
    iterateDrawcalls(texture);

    // Save out our color buffer so we can actually appreciate it
    const char* image_name = "tmp.png";
    printf("save render \"%s\"as image\n", image_name);
    // Tell the awesome STB image library that our color buffer has 4 channels
    stbi_write_png(image_name, xres, yres, 4/*channels*/, color_buffer, xres * 4/*channels*/);

    // Somewhat pointless cleanup as OS will do this for us
    free_all_textures();
    //free_mesh(&mesh);
    free(z_buffer);
    free(color_buffer);

    printf("goodbye Triangle World\n");
    return EXIT_SUCCESS;
}
