#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
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
#include "stretchy_buffer.h"
#include "camera.h"
#include "render_font/software_bitmapfont.h"

#include "vertex_shading.h"
#include "clip.h" // init frustum

// The going could get rough
// Review of structs
mat4_t normal_matrix;

struct uniforms_t
{
    mat4_t model_matrix;
    mat4_t view_matrix;
    mat4_t projection_matrix;
} uniforms;

bool sort_faces_by_z_enable = true;
bool draw_triangles_torb = true;

bool is_running = false;
unsigned int previous_frame_time = 0;

int vertex_time_start = 0;
int vertex_time_end = 0;

typedef struct {
    int x,y,z;
    float dx;
    float dy;
    float ox;
    float oy;
    bool left,right,middle;
} mouse_t;
mouse_t mouse;

mesh_t mesh;
mesh_t mesh_cube;
texture_t texture_from_file;
texture_t brick_tex;

void setup(const char* mesh_file, const char* texture_file) {

#if defined(_OPENMP)
   printf("Hello OPENMP world\n");
#else
    printf("Hello regular world\n");
#endif

    //stb__sbgrow(triangles_to_render,1024*8);

    memset(&mouse,0,sizeof(mouse));

    // Initialize render mode and triangle culling method
    render_method = RENDER_WIRE;
    cull_method = CULL_BACKFACE;

    camera.position = (vec3_t){0,0,0};

    brick_tex = load_brick_texture();
    texture_from_file = load_png_texture_data(texture_file);
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


            if (event.key.keysym.sym == SDLK_UP)
                mesh.translation.y += 0.5f;
            if (event.key.keysym.sym == SDLK_DOWN)
                mesh.translation.y -= 0.5f;
            if (event.key.keysym.sym == SDLK_LEFT)
                mesh.translation.x += 0.5f;
            if (event.key.keysym.sym == SDLK_RIGHT)
                mesh.translation.x -= 0.5f;

            if (event.key.keysym.sym == SDLK_PAGEUP)
                camera.position.z += 0.5f;
            if (event.key.keysym.sym == SDLK_PAGEDOWN)
                camera.position.z -= 0.5f;


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
            if (event.key.keysym.sym == SDLK_n)
                display_normals_enable = false;
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

static float smoothstep_inv(float A, float B, float v)
{
    //float v = i / N;
    v = 1.0f - (1.0f - v) * (1.0f - v);
    return (A * v) + (B * (1.0f - v));
}

void update(void) {
    // Wait some time until we reach the target frame time
    Uint32 sdl_time = SDL_GetTicks();
    int time_to_wait =
        (int)(previous_frame_time + FRAME_TARGET_TIME) - sdl_time;

    time_to_wait = FRAME_TARGET_TIME - (sdl_time - (int)previous_frame_time);

    // Only delay if we are too fast
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) SDL_Delay(time_to_wait);
    previous_frame_time = SDL_GetTicks();

    if (mouse.right) mesh.rotation.x += .01f*mouse.dx;
    if (mouse.right) mesh.rotation.y += .01f*mouse.dy;
    //mesh.rotation.z = 0;

    //mesh.rotation.x = .25*time;
    //mesh.rotation.z = .5*time;
    //mesh.rotation.y = -M_PI;
    //mesh.rotation.z = 0;

    if(0)//animation
    {
        //mesh.translation.x = 0;
        static float time = 0.0f;
        static float dir = 1.0f;
        //mesh.translation.y = smoothstep_inv(-1, 1, fmod(time, 1.0f) );
        //mesh.translation.y = smoothstep_inv(1, -1, time );

        if (time > 1.0)
        {
            time = 1;
            dir *= -1;
        }

        if (time < 0.0)
        {
            time = 0;
            dir *= -1;
        }
        time += dir * 0.02;
        mesh.scale.y = 1.0 + smoothstep_inv(0.0, -0.6, 0.2 + .8*time );
    }
    mesh.translation.z = -3.5f + mouse.z;



    // Initialize the target looking at the positive z-axis
    vec3_t target = { 0, 0, -1 };
    mat4_t camera_yaw_rotation = mat4_make_rotation_y(0);//camera.yaw);
    camera.direction = vec3_from_vec4(mat4_mul_vec4(camera_yaw_rotation, vec4_from_vec3(target)));

    // Offset the camera position in the direction where the camera is pointing at
    target = vec3_add(camera.position, camera.direction);
    vec3_t up_direction = { 0, 1, 0 };

    // Create the view matrix
    uniforms.view_matrix = mat4_look_at(camera.position, target, up_direction);

    // Create scale, rotation, and translation matrices that will be used to multiply the mesh vertices
    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

    // Create a World Matrix combining scale, rotation, and translation matrices
    mat4_t world_matrix = mat4_identity();

    // Order matters: First scale, then rotate, then translate. [T]*[R]*[S]*v
    world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
    world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
    world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
    world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
    world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);


    normal_matrix = world_matrix;
    uniforms.model_matrix = world_matrix;


    light.position.x = 0;
    light.position.y = 0;
    light.position.z = 0;
    light.position_proj = vec4_from_vec3(light.position);

    mat4_t light_world_matrix = mat4_identity();

    //mat4_t light_rotation_matrix_y = mat4_make_rotation_y(.5f*(float)time);
    //translation_matrix = mat4_make_translation(0,0,-6);
    //light_world_matrix = mat4_mul_mat4(translation_matrix, light_world_matrix);
    //light_world_matrix = mat4_mul_mat4(light_rotation_matrix_y, light_world_matrix);
    translation_matrix = mat4_make_translation(0,0,5);
    light_world_matrix = mat4_mul_mat4(translation_matrix, light_world_matrix);

    light.position = vec3_from_vec4( mat4_mul_vec4(light_world_matrix, light.position_proj) );

    //const mat4_t light_mvp = mat4_mul_mat4(uniforms.projection_matrix, light_world_matrix);
    //vec4_t light_transformed = mat4_mul_vec4_project(light_mvp, light.position_proj);
    //light.position_proj = to_screen_space(light_transformed);
}


static void draw_list_of_triangles(int option, int first_triangle, int last_triangle, texture_t* dc_texture)
{
    uint32_t color = packColor(255,255,255);
    triangle_t* list = get_triangles_to_render();
    for (int i = first_triangle; i < last_triangle; i++) {
        triangle_t triangle = list[i];

        // Draw filled triangle
        if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE)
        {
            light.direction = vec3_sub(light.position, triangle.center );
            vec3_normalize(&light.direction);

            vec4_t normal = vec4_from_vec3(triangle.normal);
            normal.w = 0;
            vec3_t normalInterp = vec3_from_vec4(mat4_mul_vec4(normal_matrix, normal ));
            vec3_normalize(&normalInterp);
            float n_dot_l = vec3_dot(normalInterp, light.direction);
            if (n_dot_l < 0.0f) n_dot_l = 0.0f;
            uint32_t color_lit = light_apply_intensity(color, n_dot_l);
            uint32_t colors[3] = {color_lit,color_lit,color_lit};

            draw_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, // vertex A
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, // vertex B
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, // vertex C
                colors
            );
        }

        // Draw filled triangle
        if (render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE)
        {
            light.direction = vec3_sub(light.position, triangle.center );
            vec3_normalize(&light.direction);

            vec4_t normal = vec4_from_vec3(triangle.normal);
            normal.w = 0;
            vec3_t normalInterp = vec3_from_vec4(mat4_mul_vec4(normal_matrix, normal ));
            vec3_normalize(&normalInterp);
            float n_dot_l = vec3_dot(normalInterp, light.direction);
            if (n_dot_l < 0.0f) n_dot_l = 0.0f;
            uint32_t color_lit = light_apply_intensity(color, n_dot_l);
            uint32_t colors[3] = {color_lit,color_lit,color_lit};

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
        if (    render_method == RENDER_WIRE ||
                render_method == RENDER_WIRE_VERTEX ||
                render_method == RENDER_FILL_TRIANGLE_WIRE ||
                render_method == RENDER_TEXTURED_WIRE
           ) {
            draw_triangle_lines(
                triangle.points[0].x, triangle.points[0].y, // vertex A
                triangle.points[1].x, triangle.points[1].y, // vertex B
                triangle.points[2].x, triangle.points[2].y, // vertex C
                0xFF000000
            );
        }

        // Draw triangle vertex points
        if (render_method == RENDER_WIRE_VERTEX) {
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000); // vertex A
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000); // vertex B
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000); // vertex C
        }
    }

}

void render(void) {
    clear_color_buffer( packColor(255,0,255) );
    clear_z_buffer( 1.0f );
    draw_grid();

    // We can only sort the faces within a mesh, now that all tris are one big list, need to find start offset + end
    //if (sort_faces_by_z_enable) sort_triangles();

    // Loop all projected triangles and render them
    uint32_t color = 0xFFFFFFFF;
    //if (light.position_proj.w > 0.1f)
        //circle(light.position_proj.x, light.position_proj.y, 20 - light.position_proj.z);
    circle(mouse.x, mouse.y, 10 + draw_triangles_torb*10.f);

    int raster_time_start = SDL_GetTicks();

    int num_tris = getNumTris();
    static int once=false;

    draw_call_t *drawcall_list = get_drawcall_list();
    int num_draws = array_length(drawcall_list);
    for(int i=0; i<num_draws; i++)
    {
        if(!once) {
            printf("dc %d = %p. from %d to %d \n", i, drawcall_list[i].texture->texels,  drawcall_list[i].polylist_begin, drawcall_list[i].polylist_end);

        }
        assert(drawcall_list[i].polylist_end <= num_tris);
        draw_list_of_triangles( draw_triangles_torb ? 0 : 1, drawcall_list[i].polylist_begin, drawcall_list[i].polylist_end, drawcall_list[i].texture );
    }
    if (!once) once=true;

    int raster_time_end = SDL_GetTicks();

    int vertex_time = vertex_time_end - vertex_time_start;
    int raster_time = raster_time_end - raster_time_start;
    int num_triangles_to_render = getNumTris();

    static int numframes=0;
    numframes++;

    static int frames_per_second = 0;
    static int old_time = 0;
    static int old_frames = 0;
    int time = SDL_GetTicks();
    if (old_time < time)
    {
      old_time = time + 1000;
      frames_per_second = numframes - old_frames;
      old_frames = numframes;
    }

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

    // Clear the array of triangles to render every frame loop
    freeTris();

    moveto(10,10);

    gprintf("num_draws:%d, vertexTime:%d, RasterTime:%d\n", num_draws, vertex_time, raster_time);
    gprintf("frame %d, fps:%d, culled:%d, trisRender:%d\n", numframes, frames_per_second, num_culled, num_triangles_to_render );
    gprintf("1,2,3,4,5,6, 1:wire points, 2:wire, 3:fill 4:fill wire, 5:tex, 6:tex wire\n");
    gprintf("c - cull_method=%d\n", cull_method);
    gprintf("d - disable cull=%d\n", cull_method);
    gprintf("z - sort faces by Z=%d\n", sort_faces_by_z_enable);
    gprintf("n - display_normals_enable=%d\n", display_normals_enable);
    gprintf("t - torb=%d\n", draw_triangles_torb);
    gprintf("num_culled=%d, not culled:%d, tris rendered:%d\n", num_culled, num_not_culled, num_triangles_to_render);
    gprintf("num_cull_zero_area=%d, cull_small:%d\n", num_cull_zero_area, num_cull_small_area);
    gprintf("num_cull_near=%d, far=%d, xy:%d\n", num_cull_near, num_cull_far, num_cull_xy);
    gprintf("num_cull_few=%d, many:%d\n", num_cull_few, num_cull_many);
    gprintf("num_cull_backface=%d, num_cull_degenerate(after clip)=%d\n", num_cull_backface, num_cull_degenerate);

    gprintf("l=%d, r=%d, t:%d, b:%d, n:%d, f:%d\n", cull_left, cull_right, cull_top, cull_bottom, cull_near, cull_far);

    render_color_buffer();
}

void free_resources(void) {
    free_png_texture(&texture_from_file);
    freeTris();
    free(color_buffer);
    free(z_buffer);
    free_mesh(&mesh);
    free_mesh(&mesh_cube);
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

    //mesh = load_cube_mesh_data();
    mesh = load_obj_file_data(mesh_file);
    mesh_cube = load_obj_file_data("./assets/cube.obj");

    SDL_Log("try to load %s and %s", mesh_file, texture_file);

    is_running = init_window();
    if (is_running) pk_init( color_buffer, z_buffer, get_window_width(), get_window_height() );

    setup(mesh_file, texture_file);

    while (is_running) {
        process_input();
        update();

        float aspect_y = (float)get_window_height() / (float)get_window_width();
        float aspect_x = (float)get_window_width() / (float)get_window_height();
        float fov_y = PI / 3.0; // the same as 180/3, or 60deg
        float fov_x = atan(tan(fov_y / 2.f) * aspect_x) * 2.f;
        uniforms.projection_matrix = mat4_make_perspective(fov_y, aspect_y, z_near, z_far);
        if (draw_triangles_torb == false)
        {
            init_frustum_planes(fov_x, fov_y, z_near, z_far);//, frustum_planes);
            //vertexShading(mesh, uniforms.model_matrix, uniforms.view_matrix, uniforms.projection_matrix);
        }
        else {

        }


        vertexShadingInit();

        mat4_t mvp = mat4_mul_mat4(uniforms.projection_matrix, uniforms.view_matrix);
        mvp = mat4_mul_mat4(mvp, uniforms.model_matrix);
        addDrawcall(mesh, &texture_from_file, mvp);

        int n = 2;
        for(int i=0; i<n; i++)
            for(int j=0; j<n; j++)
        {
                float x = 25*(-.5+i/(float)n);
                float z = 25*(-.5+j/(float)n);
                mat4_t model_matrix = mat4_make_translation(x,-5,-20+z);
                mat4_t mvp = mat4_mul_mat4(uniforms.projection_matrix, uniforms.view_matrix);
                mvp = mat4_mul_mat4(mvp, model_matrix);
                addDrawcall(mesh_cube, &brick_tex, mvp);
        }

        vertex_time_start = SDL_GetTicks();
        shadeDrawcalls();
        vertex_time_end = SDL_GetTicks();

        render();
    }
    destroy_window();
    free_resources();
    SDL_Log("App closed.");
    return EXIT_SUCCESS;
}
