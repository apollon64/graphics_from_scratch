#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "func.h"
#include "display.h"
#include "vecmath.h"
#include "mesh.h"
#include "array.h"
#include "matrix.h"
#include "misc.h" // string stuff
#include "light.h"
#include "texture.h"
#include "draw_triangle_pikuma.h"
#include "draw_triangle_torb.h"
#include "camera.h"
#include "render_font/software_bitmapfont.h"
#include "vertex_shading.h"
//#include "stretchy_buffer.h"
#include "clip.h" // init frustum


// Define a debug print macro
#define DEBUG_PRINT(...) \
    do { if (DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)

// Set up some globals
bool is_running = false;
unsigned int previous_frame_time = 0;
int numframes = 0;
int frames_per_second = 0;
float time = 0.0f;

// Review of structs
uniforms_t uniforms;

bool sort_faces_by_z_enable = true;
int draw_triangles_function = 1;
int vertex_shading_function = 0;
int draw_rectangles = 0;

int vertex_time_start = 0;
int vertex_time_end = 0;
int raster_time_start = 0;
int raster_time_end = 0;

typedef struct {
    int x,y,z;
    float dx;
    float dy;
    float ox;
    float oy;
    bool left,right,middle;
} mouse_t;

mouse_t mouse;

bool keyShift = false;
int key_forward = 0;
int key_down = 0;
int key_left = 0;
int key_right = 0;

mesh_t* mesh_level;
mesh_t* mesh_cube;
mesh_t* mesh_f22;
//mesh_t* mesh_sphere;
//mesh_t* mesh_plane;
texture_t texture_from_file;
//texture_t brick_tex;
texture_t texture_f22;
texture_t texture_wood;

mesh_t* mesh_sponza;
texture_t texture_sponza;

static void load_textures_and_meshes() {
    DEBUG_PRINT("SETUP!");
    memset(&mouse,0,sizeof(mouse));
    // Initialize render mode and triangle culling method
    render_method = -1;
    cull_method = CULL_BACKFACE;
    texture_from_file = load_png_texture_data("./assets/cube.png");
    texture_wood = load_png_texture_data("./assets/wood.png");
    texture_f22 = load_png_texture_data("./assets/f22.png");
    mesh_cube = mesh_from_obj("./assets/cube.obj");
    mesh_f22 = mesh_from_obj("./assets/f22.obj");
//    puts("Loading sponza");
//    texture_sponza = load_png_texture_data("./assets/sponza2_packed_full.png");
//    mesh_sponza = load_mesh_and_texture("./assets/sponza2_packed.obj");
}

static void free_resources(void) {
    freeTris();
    free_all_textures();
    free_all_meshes();
}

/**
 * @brief Processes user input to control the camera and handle application exit.
 *
 * This function checks for specific key presses and performs corresponding actions:
 * - 'ESC' key: Exits the application.
 * - 'W' key: Moves the camera forward.
 * - 'S' key: Moves the camera backward.
 * - 'A' key: Moves the camera to the left.
 * - 'D' key: Moves the camera to the right.
 * - 'Q' key: Rotates the camera to the left.
 * - 'E' key: Rotates the camera to the right.
 */
static void process_input(void) {
    SDL_Event event;
    while ( SDL_PollEvent(&event) )
    {
        switch(event.type) {
        case SDL_QUIT:
            SDL_Log("SDL_QUIT\n");
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;

            //printf("kd%d\n", event.key.keysym.sym );
            if (event.key.keysym.sym == 96)
                vertex_shading_function = 0;
            if (event.key.keysym.sym == SDLK_1) { vertex_shading_function++; if(vertex_shading_function>1) vertex_shading_function=0; }
            if (event.key.keysym.sym == SDLK_2)
                render_method = RENDER_WIRE;
            if (event.key.keysym.sym == SDLK_3)
                render_method = RENDER_FILL_TRIANGLE;
            if (event.key.keysym.sym == SDLK_4)
                render_method = RENDER_FILL_TRIANGLE_WIRE;
            if (event.key.keysym.sym == SDLK_5)
                render_method = RENDER_TEXTURED;
            if (event.key.keysym.sym == SDLK_6)
                render_method = RENDER_TEXTURED_WIRE;


//            if (event.key.keysym.sym == SDLK_UP)
//                mesh.translation.y += 0.5f;
//            if (event.key.keysym.sym == SDLK_DOWN)
//                mesh.translation.y -= 0.5f;
//            if (event.key.keysym.sym == SDLK_LEFT)
//                mesh.translation.x += 0.5f;
//            if (event.key.keysym.sym == SDLK_RIGHT)
//                mesh.translation.x -= 0.5f;

            if (event.key.keysym.sym == SDLK_PAGEUP)
                draw_rectangles++;
            if (event.key.keysym.sym == SDLK_PAGEDOWN)
                draw_rectangles--;
            if (draw_rectangles < 0) draw_rectangles = 5;
            if (draw_rectangles > 5) draw_rectangles = 0;


            if (event.key.keysym.sym == SDLK_w)
                key_forward = 1;
            if (event.key.keysym.sym == SDLK_s)
                key_down = 1;
            if (event.key.keysym.sym == SDLK_a)
                key_left = 1;
            if (event.key.keysym.sym == SDLK_d)
                key_right = 1;
            if (event.key.keysym.sym == SDLK_LSHIFT)
                keyShift = 1;
            if (event.key.keysym.sym == SDLK_t) { draw_triangles_function++; if(draw_triangles_function>2) draw_triangles_function=0; }

            break;

        case SDL_TEXTINPUT:

            if (event.key.keysym.sym == SDLK_c)
            {
                cull_method = cull_method == CULL_BACKFACE ? CULL_NONE : CULL_BACKFACE;
            }
            if (event.key.keysym.sym == SDLK_z) sort_faces_by_z_enable = !sort_faces_by_z_enable;
            if (event.key.keysym.sym == SDLK_n) display_normals_enable = true;


            break;

        case SDL_KEYUP:
        {
            render_method = -1;

            if (event.key.keysym.sym == SDLK_n)
                display_normals_enable = false;

            if (event.key.keysym.sym == SDLK_w)
                key_forward = 0;
            if (event.key.keysym.sym == SDLK_s)
                key_down = 0;
            if (event.key.keysym.sym == SDLK_a)
                key_left = 0;
            if (event.key.keysym.sym == SDLK_d)
                key_right = 0;
            if (event.key.keysym.sym == SDLK_LSHIFT)
                keyShift = 0;

            break;
        }
        case SDL_MOUSEMOTION:
        {
            mouse.x = event.motion.x;
            mouse.y = event.motion.y;

            if (event.motion.state & SDL_BUTTON_LMASK)
            {
                mouse.left = 1;
            }
            if (event.motion.state & SDL_BUTTON_RMASK)
            {
                mouse.right = 1;
            }
            if (event.motion.state & SDL_BUTTON_MMASK)
            {
                mouse.middle = 1;
            }
        }
        break;
        case SDL_MOUSEBUTTONDOWN:
        {
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                mouse.left = 1;
            }
            if (event.button.button == SDL_BUTTON_RIGHT)
            {
                mouse.right = 1;
            }
            if (event.button.button & SDL_BUTTON_MMASK)
            {
                mouse.middle = 1;
            }
        }
        break;
        case SDL_MOUSEBUTTONUP:
        {
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                mouse.left = 0;
            }
            if (event.button.button == SDL_BUTTON_RIGHT)
            {
                mouse.right = 0;
            }
            if (event.button.button & SDL_BUTTON_MMASK)
            {
                mouse.middle = 0;
            }
        }
        break;

        case SDL_MOUSEWHEEL:
        {
            if(event.wheel.y > 0) // scroll up
            {
                mouse.z--;
            }
            else if(event.wheel.y < 0) // scroll down
            {
                // Put code for handling "scroll down" here!
                mouse.z++;
            }
        }
        break;
        }
    }// while


    mouse.dx = mouse.x - mouse.ox;
    mouse.dy = mouse.y - mouse.oy;
    mouse.ox = mouse.x;
    mouse.oy = mouse.y;
}

static void sleep_until_target_fps() {
    // Wait some time until we reach the target frame time
    Uint32 sdl_time = SDL_GetTicks();
    int time_to_wait =
        (int)(previous_frame_time + FRAME_TARGET_TIME) - sdl_time;

    time_to_wait = FRAME_TARGET_TIME - (sdl_time - (int)previous_frame_time);

    // Only delay if we are too fast
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) SDL_Delay(time_to_wait);
    previous_frame_time = SDL_GetTicks();
}

void create_drawcalls(void) {
    sleep_until_target_fps();
    time = (float)klock();

    camera_controls(&camera, false/*space*/, keyShift, (key_right-key_left), (key_forward-key_down), mouse.x, mouse.y, mouse.left, 0.04);
    uniforms.view_matrix = camera.view_mat;
    uniforms.projection_matrix = camera.proj_mat;

    uniforms.color = packColorRGB(80,240,80);
    //addDrawcall( (vec3_t){0,-6,0}, (vec3_t){0,0,0}, RENDER_FILL_TRIANGLE, mesh_plane, 0x0, uniforms);

    float radi = 6;
    float anim_time = 0.f;
    float s = 2.0f;

    int N = 4;
    for(int i=0; i<N; i++)
        for(int j=0; j<N; j++)
            for(int k=0; k<N; k++)
            {
                float x = -.5f*s*(N-1) + s*i;
                float y = -.5f*s*(N-1) + s*j;
                float z = -.5f*s*(N-1) + s*k;
                vec3_t f22_pos2 = {x,y,z}; //{ cosf(-radi)*radi, cosf(-radi*2), sinf(-radi)*radi };
                vec3_t f22_rot2 = { i/(float)(N-1)*2*PI,  j/(float)(N-1)*2*PI,  k/(float)(N-1)*2*PI };
                addDrawcall(f22_pos2, f22_rot2, RENDER_TEXTURED, mesh_f22, &texture_f22, uniforms);
            }

//    vec3_t sponza_pos = {0,0,0};
//    vec3_t sponza_rot = {0,0,0};
//    addDrawcall(sponza_pos, sponza_rot, RENDER_TEXTURED, mesh_sponza, &texture_sponza, uniforms);

    uniforms.color = packColorRGB(255,32,32);
    //addDrawcall( (vec3_t){0,0,0}, (vec3_t){0,0,0}, RENDER_FILL_TRIANGLE, mesh_sphere, NULL, uniforms);

   int n = 10;
   for(int i=0; i<n; i++)
       for(int j=0; j<n; j++)
       for(int k=0; k<n; k++)
   {
           float x = n*3*(-.5+i/(float)n);
           float z = n*3*(-.5+j/(float)n);
           float y = n*3*(-.5+k/(float)n);
           uniforms.color = packColorRGB(255,255,255);
           vec3_t pos = {x,-5+y,-40+z};
           vec3_t rot = {0,0,0};
           addDrawcall(pos, rot, RENDER_TEXTURED, mesh_cube, &texture_wood, uniforms);
   }

}

static void draw_list_of_triangles(int option, draw_call_t *dc)
{
    int drawmode = dc->drawmode;
    uint32_t color = dc->uniforms.color;
    int first_triangle = dc->polylist_begin;
    int last_triangle = dc->polylist_end;
    texture_t* dc_texture = dc->texture;

    if (render_method != -1) drawmode = render_method;
    triangle_t* list = get_triangles_to_render();
    for (int i = first_triangle; i < last_triangle; i++) {
        triangle_t* triangle = &list[i];

        // Start Per Face Lighting
        //light.direction = //vec3_sub(light.position, triangle->center );
        //light.direction = vec3_sub(camera.position, triangle.center );
        vec3_normalize(&light.direction);

        //vec4_t normal = vec4_from_vec3(triangle.normal);
        //normal.w = 0;
        vec3_t normalInterp;// = triangle->normal;//vec3_from_vec4(mat4_mul_vec4(normal_matrix, normal ));
        vec3_normalize(&normalInterp);
        float n_dot_l = vec3_dot(normalInterp, light.direction);
        if (n_dot_l < 0.3) n_dot_l = 0.3f;

        uint32_t color_lit = light_apply_intensity(color, n_dot_l);
        uint32_t colors_lit[3] = {color_lit,color_lit,color_lit};

        uint32_t colors[3] = {color,color,color};
        // End Per Face Lighting


        // Draw filled triangle
        if (drawmode == RENDER_TEXTURED || drawmode == RENDER_TEXTURED_WIRE)
        {
            vertex_texcoord_t vertices[3];
            for(int vtx=0; vtx<3; vtx++)
            {
                vertices[vtx].x = triangle->points[vtx].x;
                vertices[vtx].y = triangle->points[vtx].y;
                vertices[vtx].z = triangle->points[vtx].z;
                vertices[vtx].w = triangle->points[vtx].w;
                vertices[vtx].u = triangle->texcoords[vtx].x;
                vertices[vtx].v = triangle->texcoords[vtx].y;
            }

            if (option==0)
            {
                draw_triangle_textured(vertices[0], vertices[1], vertices[2], dc_texture, colors, triangle->area2);
            }
            else if (option==1)
            {
                bizqwit_draw_triangle_textured(vertices[0], vertices[1], vertices[2], dc_texture, colors, triangle->area2);
            }
            else
            {
                draw_triangle_textured_p(vertices[0], vertices[1], vertices[2], *dc_texture);
            }
        }


        // Draw triangle wireframe
        if (    drawmode == RENDER_WIRE ||
                drawmode == RENDER_FILL_TRIANGLE_WIRE ||
                drawmode == RENDER_TEXTURED_WIRE
           ) {
            draw_triangle_lines(
                triangle->points[0].x, triangle->points[0].y, // vertex A
                triangle->points[1].x, triangle->points[1].y, // vertex B
                triangle->points[2].x, triangle->points[2].y, // vertex C
                0xFF000000
            );

            draw_rect(triangle->points[0].x - 1, triangle->points[0].y - 1, 3, 3, 0xFFFF00FF); // vertex A
            draw_rect(triangle->points[1].x - 1, triangle->points[1].y - 1, 3, 3, 0xFFFF00FF); // vertex B
            draw_rect(triangle->points[2].x - 1, triangle->points[2].y - 1, 3, 3, 0xFFFF00FF); // vertex C
        }

    }

}

static void pk_fragment_shading_step(void) {
    // Loop all projected triangles and render them

    // We can only sort the faces within a mesh, now that all tris are one big list, need to find start offset + end
    //if (sort_faces_by_z_enable) sort_triangles();

    draw_call_t *drawcall_list = get_drawcall_list();

    int num_draws = array_length(drawcall_list);
    for(int i=0; i<num_draws; i++)
    {
        draw_list_of_triangles( draw_triangles_function, &drawcall_list[i] );
    }
}

static void calc_frames_per_second()
{
    numframes++;
    static double old_time = 0;
    static int old_frames = 0;
    if (old_time < time)
    {
      old_time = time + 1.0;
      frames_per_second = numframes - old_frames;
      old_frames = numframes;
    }
}

static void draw_quad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float z, uint32_t color)
{
#define TORB
#ifdef TORB
    draw_filled_triangle(x0, y0,
                  x1, y1,
                  x2, y2, color);
    draw_filled_triangle(x0, y0,
                  x2, y2,
                  x3, y3, color);
#else
    draw_filled_triangle_p(x0, y0, x1, y1, x2, y2, color);
    draw_filled_triangle_p(x0, y0, x2, y2, x3, y3, color);
#endif
//    vertex_texcoord_t v0 = (vertex_texcoord_t){x0, y0, z, z};
//    vertex_texcoord_t v1 = (vertex_texcoord_t){x1, y1, z, z};
//    vertex_texcoord_t v2 = (vertex_texcoord_t){x2, y2, z, z};
//    vertex_texcoord_t v3 = (vertex_texcoord_t){x3, y3, z, z};

//    vec2_t v0v1 = (vec2_t){x1-x0, y1-y0};
//    vec2_t v0v2 = (vec2_t){x2-x0, y2-y0};
//    float area2;
//    area2 = fabsf((v0v1.x * v0v2.y) - (v0v1.y * v0v2.x));
//    draw_triangle_textured(v0,v1,v2,&texture_from_file,kolors,area2 );
//    draw_triangle_textured(v0,v2,v3,&texture_from_file,kolors,area2 );
}

void draw_triangle_grid() {
    // setups grid stored in gridpoints and mesh
#define GRID_SIZE 22
    static vec3_t grid[(GRID_SIZE+1)*(GRID_SIZE+1)];
    static vec3_t mesh[2*GRID_SIZE*GRID_SIZE][3];
    float startx=0.f, starty=0.f;
    float s = pk_window_width();
    srand(0);
    for(int i=0; i<GRID_SIZE+1; i++)
    for(int j=0; j<GRID_SIZE+1; j++) {
       float x = startx+s*j/GRID_SIZE -10+30*frand() ;// + 2.*noise(xs+.5*animation,ys);
       float y = starty+s*i/GRID_SIZE -10+30*frand();// + 2.*noise(ys+.5*animation,2*xs);
       grid[j*(GRID_SIZE+1)+i] = (vec3_t){x,y,0};
    }
    int grid_points[3];
    for(int i=0; i<GRID_SIZE; i++)
    for(int j=0; j<GRID_SIZE; j++)
    {
       int idx0 = 2*(i*GRID_SIZE+j)+0;
       int idx1 = 2*(i*GRID_SIZE+j)+1;

       int g00 = j*(GRID_SIZE+1)+i;
       int g10 = j*(GRID_SIZE+1)+(i+1);
       int g01 = (j+1)*(GRID_SIZE+1)+i;
       int g11 = (j+1)*(GRID_SIZE+1)+(i+1);
       grid_points[0] = g00;
       grid_points[1] = g01;
       grid_points[2] = g11;
       int gidx=0; // use this to rotate triangles
       mesh[idx0][0] = grid[grid_points[(gidx+0)%3]];
       mesh[idx0][1] = grid[grid_points[(gidx+1)%3]];
       mesh[idx0][2] = grid[grid_points[(gidx+2)%3]];

       grid_points[0] = g00;
       grid_points[1] = g11;
       grid_points[2] = g10;
       gidx=1; // use this to rotate triangles
       mesh[idx1][0] = grid[grid_points[(gidx+0)%3]];
       mesh[idx1][1] = grid[grid_points[(gidx+1)%3]];
       mesh[idx1][2] = grid[grid_points[(gidx+2)%3]];

       draw_filled_triangle(
               mesh[idx0][0].x, mesh[idx0][0].y,
               mesh[idx0][1].x, mesh[idx0][1].y,
               mesh[idx0][2].x, mesh[idx0][2].y,
               packColorRGBAf(0.f, 1.f, 0.f, 1.f));

       draw_filled_triangle(
               mesh[idx1][0].x, mesh[idx1][0].y,
               mesh[idx1][1].x, mesh[idx1][1].y,
               mesh[idx1][2].x, mesh[idx1][2].y,
               packColorRGBAf(1.f, 0.f, 0.f, 1.f));
    }
}

void draw_full_screen_quad()
{
    switch (draw_rectangles)
    {
        case 1: clear_color_buffer(packColorRGBA(0, 163, 232, 0)); break;
        case 2:
            for (int y = 0; y < pk_window_height(); y++)
                for (int x = 0; x < pk_window_width(); x++)
                {
                    setpix_no_bound_check(x, y, packColorRGBA(0, 163, 232, 0));
                }
            break;
        case 3:
            clear_color_buffer(packColorRGBA(0, 163, 232, 0));
            break;
        case 4:
        {
            uint32_t val = packColorRGBA(0, 163, 232, 0);
            int wxh = pk_window_width() * pk_window_height();
            for (int j = 0; j < wxh; j++)
                color_buffer[j] = val;
            break;
        }
        case 5:
        {
            draw_quad(0.f, 0.f, pk_window_width(), 0.f, pk_window_width(), pk_window_height(), 0.f, pk_window_height(), 0.0f, packColorRGBA(0, 163, 232, 0) );
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    (void)&argc;
    (void)&argv;
    SDL_Log("Hello courses.pikuma.com ! Build %s at %s\n", __DATE__, __TIME__);
#if DEBUG==0
    puts("DBG = 0");
#endif
#if DEBUG==1
       puts("DBG = 1");
#endif

    DEBUG_PRINT("Boot\n");

#ifdef MY_COMPILER
    printf("MY_COMPILER=%s\n", MY_COMPILER);
#endif

    load_textures_and_meshes();

    is_running = open_sdl_window();
    alloc_framebuffer();
    if (is_running) pk_init( color_buffer, z_buffer, get_window_width(), get_window_height() );

    float aspect_y = (float)get_window_height() / (float)get_window_width();
    float aspect_x = (float)get_window_width() / (float)get_window_height();
    float fov_y = PI / 3.0; // the same as 180/3, or 60deg
    float fov_x = atan(tan(fov_y / 2.f) * aspect_x) * 2.f;
    camera = camera_init((vec3_t){3,7,-2}, 0,0,fov_y, aspect_y, z_near, z_far);
    camera.posv = 3.14159/4.f;
    camera.posh = -3.14159/4.f;
    init_frustum_planes(fov_x, fov_y, z_near, z_far);

    while (is_running) {
        process_input();
        clearDrawcalls();
        create_drawcalls();
        vertex_time_start = SDL_GetTicks();
            pk_vertex_shading_step(vertex_shading_function);
        vertex_time_end = SDL_GetTicks();

        raster_time_start = SDL_GetTicks();

        clear_color_buffer( 0u ); //clear_color_buffer(packColorRGBA(0, 163, 232, 0));
        clear_z_buffer( 1.0f );
        //draw_full_screen_quad();
        pk_fragment_shading_step();


        uint32_t color = 0xFFFFFFFF;
        circle(mouse.x, mouse.y, 10 + draw_triangles_function*5.f);
        if (display_normals_enable)
        {
            color = 0xFF00FF00;
            int num_lines = array_length(get_lines_to_render());
            for (int i = 0; i < num_lines; i++) {
                line_t line = get_lines_to_render()[i];
                draw_line3d(line.a.x, line.a.y, line.a.w, line.b.x, line.b.y, line.b.w, color);
            }

        }


        //draw_triangle_grid();
        raster_time_end = SDL_GetTicks();

        moveto(10,10);

        int vertex_time = vertex_time_end - vertex_time_start;
        int raster_time = raster_time_end - raster_time_start;
        static int raster_max = 0;
        static int raster_min = 1000;
        if (raster_time > raster_max) raster_max = raster_time;
        if (raster_time < raster_min) raster_min = raster_time;
        int num_triangles_to_render = getNumTris();
        setfont(8, 16);

        draw_quad(0.f, 0.f, pk_window_width(), 0.f,
                  pk_window_width(), pk_window_height()/2, 0.f, pk_window_height()/2,
                  0.0f, // z
                  packColorRGBA(128, 128, 128, 128) );
        gprintf("vertexTime:%4d, RasterTime:%4d, [%4d, %4d]\n", vertex_time, raster_time, raster_min, raster_max);
        gprintf("frame %d, fps:%d, tris_issued[%d] tris_culled[%d], trisRender:%d\n", numframes, frames_per_second, vss.num_triangles_issued, vss.num_triangles_culled, num_triangles_to_render );
        gprintf("1,2,3,4,5,6, 1:wire points, 2:wire, 3:fill 4:fill wire, 5:tex, 6:tex wire\n");
        gprintf("c - cull_method=%d\n", cull_method);
        gprintf("z - sort faces by Z=%d\n", sort_faces_by_z_enable);
        gprintf("n - display_normals_enable=%d\n", display_normals_enable);
        gprintf("t - draw_triangles_function=%d (0=t 1=b, 2=p)\n", draw_triangles_function);


        int show_vertex_stats = 1;
        if (show_vertex_stats)
        {
            setfont(8, 8);
            gprintf("---------VERTEX STATS func%d ------\n", vertex_shading_function);
            gprintf("%d num_triangles_issued\n", vss.num_triangles_issued);
            gprintf("%d num_triangles_to_render\n", num_triangles_to_render);
            gprintf("%d num_triangles_culled\n", vss.num_triangles_culled);
            gprintf("%d num_cull_backface\n", vss.num_cull_backface);
            gprintf("%d num_cull_zero_area\n", vss.num_cull_zero_area);
            gprintf("%d num_cull_small_area\n", vss.num_cull_small_area);
            gprintf("%d num_cull_degenerate\n", vss.num_cull_degenerate);
            gprintf("%d num_cull_near\n", vss.num_cull_near);
            gprintf("%d num_cull_far\n", vss.num_cull_far);
            gprintf("%d num_cull_xy\n", vss.num_cull_xy);
            gprintf("%d num_clip_output_degen\n", vss.num_clip_output_degen);
        }

        gprintf("cam: %.1f, %.1f, %.1f,   %.1f, %.1f\n", camera.position.x, camera.position.y, camera.position.z, camera.posh, camera.posv);
        gprintf("draw_rectangles:%d (pgupdown) 0:off, 1:clear_color_buffer 2:setpix, 3:clear xN, 4:memset, 5:draw 2 tris\n", draw_rectangles);
        unsigned pixel_count = pk_window_width() * pk_window_height();
        double   megapixels = (double)(pixel_count) / 1e6f;
        unsigned megs = (pixel_count * sizeof(uint32_t)) / 1024 / 1024; // Pixel is 32-bit, 4 byte
        gprintf("1 blit fillrate=%f MPIX/frame, megs_per_Frame=%d\n", megapixels, megs );
        pk_blit_color_to_screen();

        calc_frames_per_second();
    }
    destroy_window();
    free_resources();
    SDL_Log("App closed.");
    return EXIT_SUCCESS;
}
