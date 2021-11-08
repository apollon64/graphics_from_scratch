#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

#include "func.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"
#include "matrix.h"
#include "misc.h" // string stuff
#include "light.h"
#include "texture.h"
#include "draw_triangle_pikuma.h"
#include "draw_triangle_torb.h"
//#include "stretchy_buffer.h"
#include "camera.h"
#include "render_font/software_bitmapfont.h"

#include "vertex_shading.h"
#include "clip.h" // init frustum

// The going could get rough
bool is_running = false;
unsigned int previous_frame_time = 0;
int numframes = 0;
int frames_per_second = 0;
float time = 0.0f;

// Review of structs
uniforms_t uniforms;

bool sort_faces_by_z_enable = true;
bool draw_triangles_torb = true;
bool toggle_me_beautiful = true;

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

mesh_t* mesh;
mesh_t* mesh_cube;
mesh_t* mesh_f22;
mesh_t* mesh_sphere;
mesh_t* mesh_plane;
texture_t texture_from_file;
texture_t brick_tex;
texture_t texture_f22;

void setup(const char* mesh_file, const char* texture_file) {
    memset(&mouse,0,sizeof(mouse));

    // Initialize render mode and triangle culling method
    render_method = -1;
    cull_method = CULL_BACKFACE;

//    mesh = load_mesh_and_texture(mesh_file);
//    texture_from_file = load_png_texture_data(texture_file);

    mesh = load_mesh_and_texture("./assets/wfbake.obj");
    texture_from_file = load_png_texture_data("./assets/wf512.png");

    mesh_cube = load_mesh_and_texture("./assets/box.obj");
    mesh_f22 = load_mesh_and_texture("./assets/f22.obj");
    mesh_sphere = load_mesh_and_texture("./assets/sphere.obj");
    mesh_plane = load_mesh_and_texture("./assets/plane.obj");

    brick_tex = load_png_texture_data("assets/brik.png");

    texture_f22 = load_png_texture_data("./assets/f22.png");
}

void free_resources(void) {
    freeTris();

    free_all_textures();
    free_all_meshes();
}


uint32_t vec3_to_uint32_t(vec3_t c)
{
    return packColor( (U8)c.x*255, (U8)c.y*255, (U8)c.z*255);
}

void process_input(void) {
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

            if (event.key.keysym.sym == SDLK_1)
                render_method = RENDER_WIRE_VERTEX;
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
                toggle_me_beautiful = true;
            if (event.key.keysym.sym == SDLK_PAGEDOWN)
                toggle_me_beautiful = false;

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



        case SDL_TEXTINPUT:

            if (event.key.keysym.sym == SDLK_c)
            {
                cull_method = cull_method == CULL_BACKFACE ? CULL_NONE : CULL_BACKFACE;
            }
            if (event.key.keysym.sym == SDLK_z) sort_faces_by_z_enable = !sort_faces_by_z_enable;
            if (event.key.keysym.sym == SDLK_n) display_normals_enable = true;

            if (event.key.keysym.sym == SDLK_t) draw_triangles_torb = !draw_triangles_torb;
            if (event.key.keysym.sym == SDLK_r)
            {
                draw_triangles_torb = false;
            }
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

void sleep_until_target_fps() {
    // Wait some time until we reach the target frame time
    Uint32 sdl_time = SDL_GetTicks();
    int time_to_wait =
        (int)(previous_frame_time + FRAME_TARGET_TIME) - sdl_time;

    time_to_wait = FRAME_TARGET_TIME - (sdl_time - (int)previous_frame_time);

    // Only delay if we are too fast
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) SDL_Delay(time_to_wait);
    previous_frame_time = SDL_GetTicks();
}

void update(void) {
    sleep_until_target_fps();
    time = (float)klock();

    camera_controls(&camera, false/*space*/, keyShift, (key_right-key_left), (key_forward-key_down), mouse.x, mouse.y, mouse.left, 0.04);
    uniforms.view_matrix = camera.view_mat;
    uniforms.projection_matrix = camera.proj_mat;

//    for (int i=0; i<5; i++)
//    for (int j=0; j<5; j++)
//    {
//        addDrawcall((vec3_t){i*60,0,-j*60}, (vec3_t){0,0,0}, RENDER_TEXTURED, mesh, &texture_from_file, uniforms);
//    }
    addDrawcall((vec3_t){0,0,0}, (vec3_t){0,0,0}, RENDER_TEXTURED, mesh, &texture_from_file, uniforms);

    uniforms.color = packColor(80,240,80);
    //addDrawcall( (vec3_t){0,-6,0}, (vec3_t){0,0,0}, RENDER_FILL_TRIANGLE, mesh_plane, 0x0, uniforms);

    float radi = 4;
    vec3_t f22_pos = { cosf(time)*radi, cosf(time*2), sinf(time)*radi };
    vec3_t f22_rot = { time*2, -time-.5*PI, 0 };
    addDrawcall(f22_pos, f22_rot, RENDER_TEXTURED, mesh_f22, &texture_f22, uniforms);

    vec3_t f22_pos2 = { cosf(-time)*radi, cosf(-time*2), sinf(-time)*radi };
    vec3_t f22_rot2 = { 0, time + .5f * PI, 0 };
    addDrawcall(f22_pos2, f22_rot2, RENDER_TEXTURED, mesh_f22, &texture_f22, uniforms);

    uniforms.color = packColor(255,32,32);
    addDrawcall( (vec3_t){0,0,0}, (vec3_t){0,0,0}, RENDER_FILL_TRIANGLE, mesh_sphere, NULL, uniforms);

//    int n = 20;
//    for(int i=0; i<n; i++)
//        for(int j=0; j<n; j++)
//    {
//            float x = n*3*(-.5+i/(float)n);
//            float z = n*3*(-.5+j/(float)n);
//            uniforms.color = packColor(255,255,255);
//            addDrawcall((vec3_t){x,-5,-20+z}, (vec3_t){0,0,0}, RENDER_TEXTURED, mesh_cube, &brick_tex, uniforms);
//    }

}

float maxf(float a, float b) { return a > b ? a : b; }

static void draw_list_of_triangles(int option, int drawmode, uint32_t color, int first_triangle, int last_triangle, texture_t* dc_texture)
{
    //vec3_t objectColor;
    //unpackColor(color, &objectColor.x, &objectColor.y, &objectColor.z);

    if (render_method != -1) drawmode = render_method;
    triangle_t* list = get_triangles_to_render();
    for (int i = first_triangle; i < last_triangle; i++) {
        triangle_t triangle = list[i];
        //dc_texture = triangle.tex_hand == 0 ? 0x0 : find_texture_by_handle(triangle.tex_hand);
        //drawmode = triangle.tex_hand == 0 ? RENDER_FILL_TRIANGLE : RENDER_TEXTURED;



        // Start Per Face Lighting
        light.direction = vec3_sub(light.position, triangle.center );
        //light.direction = vec3_sub(camera.position, triangle.center );
        vec3_normalize(&light.direction);

        //vec4_t normal = vec4_from_vec3(triangle.normal);
        //normal.w = 0;
        vec3_t normalInterp = triangle.normal;//vec3_from_vec4(mat4_mul_vec4(normal_matrix, normal ));
        vec3_normalize(&normalInterp);
        float n_dot_l = vec3_dot(normalInterp, light.direction);
        if (n_dot_l < 0.3) n_dot_l = 0.3f;

        uint32_t color_lit = light_apply_intensity(color, n_dot_l);
        uint32_t colors_lit[3] = {color_lit,color_lit,color_lit};

        uint32_t colors[3] = {color,color,color};
        // End Per Face Lighting

        // Draw filled triangle
        if (drawmode == RENDER_FILL_TRIANGLE || drawmode == RENDER_FILL_TRIANGLE_WIRE)
        {
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, // vertex A
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, // vertex B
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, // vertex C
                colors
            );
        }

        // Draw filled triangle
        if (drawmode == RENDER_TEXTURED || drawmode == RENDER_TEXTURED_WIRE)
        {
            vertex_texcoord_t vertices[3];
            for(int vtx=0; vtx<3; vtx++)
            {
                vertices[vtx].x = triangle.points[vtx].x;
                vertices[vtx].y = triangle.points[vtx].y;
                vertices[vtx].z = triangle.points[vtx].z;
                vertices[vtx].w = triangle.points[vtx].w;
                vertices[vtx].u = triangle.texcoords[vtx].x;
                vertices[vtx].v = triangle.texcoords[vtx].y;
            }

            if (option==0)
            {
                draw_triangle_textured(vertices[0], vertices[1], vertices[2], dc_texture, colors, triangle.area2);
            }
            else
            {
                draw_triangle_textured_p(vertices[0], vertices[1], vertices[2], *dc_texture);
            }
        }


        // Draw triangle wireframe
        if (    drawmode == RENDER_WIRE ||
                drawmode == RENDER_WIRE_VERTEX ||
                drawmode == RENDER_FILL_TRIANGLE_WIRE ||
                drawmode == RENDER_TEXTURED_WIRE
           ) {
            draw_triangle_lines(
                triangle.points[0].x, triangle.points[0].y, // vertex A
                triangle.points[1].x, triangle.points[1].y, // vertex B
                triangle.points[2].x, triangle.points[2].y, // vertex C
                0xFF000000
            );
        }

        // Draw triangle vertex points
        if (drawmode == RENDER_WIRE_VERTEX) {
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000); // vertex A
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000); // vertex B
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000); // vertex C
        }
    }

}

void iterateDrawcalls(void) {
    // Loop all projected triangles and render them

    // We can only sort the faces within a mesh, now that all tris are one big list, need to find start offset + end
    //if (sort_faces_by_z_enable) sort_triangles();

    draw_call_t *drawcall_list = get_drawcall_list();

    int num_draws = array_length(drawcall_list);
    for(int i=0; i<num_draws; i++)
    {
        int t = draw_triangles_torb ? 0 : 1;
        draw_list_of_triangles( t, drawcall_list[i].drawmode, drawcall_list[i].uniforms.color, drawcall_list[i].polylist_begin, drawcall_list[i].polylist_end, drawcall_list[i].texture );
    }


    // we could avoid iterating through drawcalls if all drawcall-specific fields were stored with each triangle,
    // but that increases the size of the triangle structure uneededly (?)
    // instead of the loop here, we would do:
    // draw_list_of_triangles( draw_triangles_torb ? 0 : 1, RENDER_TEXTURED, uniforms.color, 0, getNumTris() , 0x0 );

    clearDrawcalls();

}

void calc_frames_per_second()
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

void draw_quad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float z, uint32_t color)
{
    uint32_t kolors[3];
    kolors[0] = color;
    draw_triangle(x0, y0, z, z,
                  x1, y1, z, z,
                  x2, y2, z, z, kolors);
    draw_triangle(x0, y0, z, z,
                  x2, y2, z, z,
                  x3, y3, z, z, kolors);

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

int main(int argc, char *argv[])
{
    SDL_Log("Hello courses.pikuma.com ! Build %s at %s\n", __DATE__, __TIME__);
    const char* mesh_file = "./assets/cube.obj";
    const char* texture_file = "./assets/cube.png";
    for(int i=0; i<argc; i++)
    {
        if (i==0) continue;
        if (EndsWith(argv[i], "obj") ) {
            mesh_file = argv[i];
        }
        if (EndsWith(argv[i], "png") ) {
            texture_file = argv[i];
        }
    }


    SDL_Log("try to load %s and %s", mesh_file, texture_file);
    setup(mesh_file, texture_file);

    is_running = init_window();
    if (is_running) pk_init( color_buffer, z_buffer, get_window_width(), get_window_height() );

    float aspect_y = (float)get_window_height() / (float)get_window_width();
    float aspect_x = (float)get_window_width() / (float)get_window_height();
    float fov_y = PI / 3.0; // the same as 180/3, or 60deg
    float fov_x = atan(tan(fov_y / 2.f) * aspect_x) * 2.f;
    //camera = camera_init((vec3_t){0,0,-50}, 0,0,fov_y, aspect_y, z_near, z_far);
    camera = camera_init((vec3_t){-32, 32, 33 }, 2.8, 0.3, fov_y, aspect_y, z_near, z_far);

    init_frustum_planes(fov_x, fov_y, z_near, z_far);//, frustum_planes);

    while (is_running) {
        process_input();
        update();

        vertex_time_start = SDL_GetTicks();
        shadeDrawcalls(draw_triangles_torb ? 1 : 0);
        vertex_time_end = SDL_GetTicks();

        raster_time_start = SDL_GetTicks();

        if (toggle_me_beautiful)
        {
            clear_color_buffer( packColor(0,163,232) );
        } else
        {
            int lim = pk_window_width() / 10 / 2;
            int n = 10;
            for (float i=0; i<n; i++)
            {
                float zeroTo1 = i / (float)n;
                float s = 20.0f;
                if (i*s > pk_window_width()/2 ) break;
                draw_quad(i*s,i*s, pk_window_width()-i*s, i*s, pk_window_width()-i*s, pk_window_height()-i*s, i*s, pk_window_height()-i*s,
                          1.f - zeroTo1, packColor(((int)(i)*30)%255,163,((int)i*100)%255) );
            }
            //draw_quad(0,0, pk_window_width(), 0, pk_window_width(), pk_window_height(), 0, pk_window_height(), packColor(0,163,232) );
        }


        clear_z_buffer( 1.0f );
        draw_grid();

        uint32_t color = 0xFFFFFFFF;
        //if (light.position_proj.w > 0.1f)
            //circle(light.position_proj.x, light.position_proj.y, 20 - light.position_proj.z);
        circle(mouse.x, mouse.y, 10 + draw_triangles_torb*10.f);
        if (display_normals_enable)
        {
            color = 0xFF00FF00;
            int num_lines = array_length(get_lines_to_render());
            for (int i = 0; i < num_lines; i++) {
                line_t line = get_lines_to_render()[i];
                draw_line3d(line.a.x, line.a.y, line.a.w, line.b.x, line.b.y, line.b.w, color);
            }

        }
        //draw_triangle(300, 100, 50, 400, 500, 700, 0xFF00FF00);
        draw_triangle_textured_p(
            (vertex_texcoord_t){ 0,   get_window_height()-0, 0.0f,   1.0f, 0.0f, 0.0f },
            (vertex_texcoord_t){ 100, get_window_height()-100, 0.0f, 1.0f, 1.0f, 1.0f},
            (vertex_texcoord_t){ 100, get_window_height()-00, 0.0f,  1.0f, 1.0f, 0.0f},
            texture_from_file
        );

        iterateDrawcalls();
        raster_time_end = SDL_GetTicks();

        moveto(10,10);

        int vertex_time = vertex_time_end - vertex_time_start;
        int raster_time = raster_time_end - raster_time_start;
        static int raster_max = 0;
        static int raster_min = 1000;
        if (raster_time > raster_max) raster_max = raster_time;
        if (raster_time < raster_min) raster_min = raster_time;
        int num_triangles_to_render = getNumTris();
        gprintf("vertexTime:%4d, RasterTime:%4d, [%4d, %4d]\n", vertex_time, raster_time, raster_min, raster_max);

        gprintf("frame %d, fps:%d, culled:%d, trisRender:%d\n", numframes, frames_per_second, num_culled, num_triangles_to_render );
        gprintf("1,2,3,4,5,6, 1:wire points, 2:wire, 3:fill 4:fill wire, 5:tex, 6:tex wire\n");
        gprintf("c - cull_method=%d\n", cull_method);
        gprintf("z - sort faces by Z=%d\n", sort_faces_by_z_enable);
        gprintf("n - display_normals_enable=%d\n", display_normals_enable);
        gprintf("t - torb=%d\n", draw_triangles_torb);
        gprintf("num_culled=%d, not culled:%d, tris rendered:%d\n", num_culled, num_not_culled, num_triangles_to_render);
        gprintf("num_cull_zero_area=%d, cull_small:%d\n", num_cull_zero_area, num_cull_small_area);
        gprintf("num_cull_near=%d, far=%d, xy:%d\n", num_cull_near, num_cull_far, num_cull_xy);
        gprintf("num_cull_few=%d, many:%d\n", num_cull_few, num_cull_many);
        gprintf("num_cull_backface=%d, num_cull_degenerate(after clip)=%d\n", num_cull_backface, num_cull_degenerate);

        gprintf("cam: %.1f, %.1f, %.1f,   %.1f, %.1f\n", camera.position.x, camera.position.y, camera.position.z, camera.posh, camera.posv);

        gprintf("l=%d, r=%d, t:%d, b:%d, n:%d, f:%d\n", cull_left, cull_right, cull_top, cull_bottom, cull_near, cull_far);
        gprintf("toggle:%d\n", toggle_me_beautiful);

        render_color_buffer();

        calc_frames_per_second();
    }
    destroy_window();
    free_resources();
    SDL_Log("App closed.");
    return EXIT_SUCCESS;
}

