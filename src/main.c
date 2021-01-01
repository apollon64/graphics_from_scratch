#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include <math.h>
#include <string.h>
#include "func.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"

// Review of structs
typedef struct {
    vec3_t position;
    vec3_t rotation;
    float fov_angle;
} camera_t;

typedef struct {
    vec3_t a,b;
} line_t;

bool sort_faces_by_z_enable = true;
bool display_normals_enable = false;
triangle_t *triangles_to_render = NULL;
line_t* lines_to_render = NULL;

camera_t camera = {
    {0, 0, 0},
    {0.3, -2.0, 0.0},
    0.78
};
double time = 0;

float fov_factor = 640;
bool is_running = false;
unsigned int previous_frame_time = 0;

struct mouse_t {
    int x,y;
    bool left,right;
} mouse;

void setup(const char* mesh_file) {
    time = 0.0;

    // Initialize render mode and triangle culling method
    render_method = RENDER_WIRE;
    cull_method = CULL_BACKFACE;

    //load_cube_mesh_data();
    load_obj_file_data(mesh_file);
}

vec2_t project(vec3_t point) {
    vec2_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z
    };
    return projected_point;
}

vec3_t project3d(vec3_t point) {
    vec3_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z,
        .z = point.z
    };
    return projected_point;
}

void process_input(void) {
    SDL_Event event;
    //SDL_PollEvent(&event);
    while ( SDL_PollEvent(&event) )
    {
        //mouse.left = 0;
        //mouse.right = 0;

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
            if (event.key.keysym.sym == SDLK_c)
                cull_method = CULL_BACKFACE;
            if (event.key.keysym.sym == SDLK_d)
                cull_method = CULL_NONE;
            if (event.key.keysym.sym == SDLK_z)
                sort_faces_by_z_enable = true;
            if (event.key.keysym.sym == SDLK_x)
                sort_faces_by_z_enable = false;
            if (event.key.keysym.sym == SDLK_n)
                display_normals_enable = true;
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
        }
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
        }

        break;
        }
    }// while
}

void update(void) {

    // Wait some time until we reach the target frame time
    int time_to_wait =
        (int)(previous_frame_time + FRAME_TARGET_TIME) - SDL_GetTicks();

    time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - (int)previous_frame_time);

    // Only delay if we are too fast
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) SDL_Delay(time_to_wait);
    previous_frame_time = SDL_GetTicks();

    // Initialize the array of triangles to render
    triangles_to_render = NULL;

    time += 0.01;
    mesh.rotation.x = 3.14;
    mesh.rotation.y = mouse.x / (float)window_width * 2*M_PI;
    mesh.rotation.z += 0.0;

    lines_to_render = NULL;

    float translateY = 1.5f;
    float translateZ = 5.5f;

    // Loop all triangle faces of our mesh
    int n_faces = array_length(mesh.faces);
    for (int i = 0; i < n_faces; i++) {
        face_t mesh_face = mesh.faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1]; // Minus 1 because mesh vertices start from 1.
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        vec3_t center = {0,0,0};
        center = vec3_add(center, face_vertices[0]);
        center = vec3_add(center, face_vertices[1]);
        center = vec3_add(center, face_vertices[2]);
        center = vec3_mul(center, 1.0 / 3.0f);
        center = vec3_rotate_x(center, mesh.rotation.x);
        center = vec3_rotate_y(center, mesh.rotation.y);
        center = vec3_rotate_z(center, mesh.rotation.z);

        center.y += translateY;
        center.z += translateZ;


        vec3_t transformed_vertices[3];
        // Loop all three vertices of this current face and apply transformations
        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];

            transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            // Translate the vertex away from the camera
            transformed_vertex.y += translateY;
            transformed_vertex.z += translateZ;
            transformed_vertices[j] = transformed_vertex;
        }

        triangle_t projected_triangle;
        projected_triangle.z = center.z;
        for (int j = 0; j < 3; j++) {
            vec2_t projected_point = project(transformed_vertices[j]);
            // Scale and translate the projected points to the middle of the screen
            projected_point.x += (window_width / 2);
            projected_point.y += (window_height / 2);
            projected_triangle.points[j] = projected_point;
        }

        // Check backface culling
        vec3_t vector_ab = vec3_sub(transformed_vertices[1], transformed_vertices[0]);
        vec3_t vector_ac = vec3_sub(transformed_vertices[2], transformed_vertices[0]);
        vec3_t normal = vec3_cross(vector_ab, vector_ac);

        // Find the vector between a point in the triangle and camera origin
        vec3_t camera_ray = vec3_sub(camera.position, transformed_vertices[0]);

        float rayDotNormal = vec3_dot(camera_ray, normal);

        // Bypass the triangles looking away from camera
        bool front_facing = rayDotNormal > 0.0f;

        //if (cull_method == CULL_BACKFACE && front_facing)
        {
            // Save the projected triangle in the array of triangles to render
            if (cull_method == CULL_BACKFACE && !front_facing)
            {
                continue;
            }
            array_push(triangles_to_render, projected_triangle);


            vec3_t start = project3d(center);
            start.x += (window_width / 2);
            start.y += (window_height / 2);

            vec3_t normal = vec3_cross(
                                vec3_sub(face_vertices[1],face_vertices[0]),
                                vec3_sub(face_vertices[2],face_vertices[0])
                            );
            normal = vec3_rotate_x(normal, mesh.rotation.x);
            normal = vec3_rotate_y(normal, mesh.rotation.y);
            normal = vec3_rotate_z(normal, mesh.rotation.z);

            vec3_normalize(&normal);
            normal = vec3_mul(normal, 0.125f);
            vec3_t end = project3d( vec3_add(center, normal) );
            end.x += (window_width / 2);
            end.y += (window_height / 2);

            line_t projected_line = {.a = start, .b = end};
            array_push(lines_to_render, projected_line);
        }


    }
}

bool getWinding(float x0, float y0, float x1, float y1, float x2, float y2)
{
    vec2_t v0 = {x1-x0, y1-y0};
    vec2_t v1 = {x2-x0, y2-y0};
    float dot= v0.x*v1.x + v0.y*v1.y;
    if (dot > 0) return true;
    return false;
}


/* this function will be used by qsort to compare elements */
int cmp(const void *v1, const void *v2) {
    triangle_t f1=*((triangle_t*)v1);
    triangle_t f2=*((triangle_t*)v2);
    if(f1.z < f2.z)
        return -1;
    else if(f1.z > f2.z)
        return 1;
    return 0;
}

int cmpLess(const void *triangleA, const void *triangleB) {
    triangle_t a=*((triangle_t*)triangleA);
    triangle_t b=*((triangle_t*)triangleB);
    if(a.z > b.z)
        return -1;
    else if(a.z < b.z)
        return 1;
    return 0;
}

void sort(void)
{
    int num_tris = array_length(triangles_to_render);
    qsort(triangles_to_render, num_tris, sizeof(triangle_t), cmpLess);
}

void render(void) {

    clear_color_buffer( 0xFF000000 );
    draw_grid();

    vec3_t centerPos = {0,0,0};
    vec3_t camera_dir =
        vec3_sub(camera.position, centerPos);

    vec3_normalize(&camera_dir);

    if (sort_faces_by_z_enable) sort();

    // Loop all projected triangles and render them
    uint32_t color = 0xFFFFFFFF;
    int num_tris = array_length(triangles_to_render);
    for (int i = 0; i < num_tris; i++) {
        triangle_t triangle = triangles_to_render[i];

        // Draw filled triangle
        if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE) {
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y, // vertex A
                triangle.points[1].x, triangle.points[1].y, // vertex B
                triangle.points[2].x, triangle.points[2].y, // vertex C
                0xFF555555
            );
        }

        // Draw triangle wireframe
        if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE) {
            draw_triangle_lines(
                triangle.points[0].x, triangle.points[0].y, // vertex A
                triangle.points[1].x, triangle.points[1].y, // vertex B
                triangle.points[2].x, triangle.points[2].y, // vertex C
                0xFFFFFFFF
            );
        }

        // Draw triangle vertex points
        if (render_method == RENDER_WIRE_VERTEX) {
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000); // vertex A
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000); // vertex B
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000); // vertex C
        }
    }

    if (display_normals_enable)
    {
        color = 0xFF00FF00;
        int num_lines = array_length(lines_to_render);
        for (int i = 0; i < num_lines; i++) {
            line_t line = lines_to_render[i];
            draw_line(line.a.x, line.a.y, line.b.x, line.b.y, color);
        }

    }


    //draw_triangle(300, 100, 50, 400, 500, 700, 0xFF00FF00);

    // Clear the array of triangles to render every frame loop
    array_free(lines_to_render);
    array_free(triangles_to_render);

    render_color_buffer();
}

void free_resources(void) {
    free(color_buffer);
    array_free(mesh.vertices);
    array_free(mesh.faces);
}

int EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

int main(int argc, char *argv[])
{
    SDL_Log("Hello courses.pikuma.com\n");
    for(int i=0; i<argc; i++)
    {
        SDL_Log("%d:%s\n", i, argv[i]);
    }
    const char* mesh_file = NULL;
    if (argc > 1 && EndsWith(argv[1], "obj") )
    {
        mesh_file = argv[1];
        //mesh_file = "./assets/f22.obj";
    } else {
        mesh_file = "./assets/cube.obj";
    }
    SDL_Log("try to load %s", mesh_file);
    is_running = init_window();
    setup(mesh_file);
    while (is_running) {
        process_input();
        update();
        render();
    }
    destroy_window();
    free_resources();
    SDL_Log("App closed.");
    return EXIT_SUCCESS;
}
