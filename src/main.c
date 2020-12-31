#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include <math.h>
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

triangle_t *triangles_to_render = NULL;

camera_t camera = {
    {0,0,-5.f},
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

void setup(void) {
    time = 0.0;
    //load_cube_mesh_data();
    load_obj_file_data("./assets/f22.obj");
}

vec2_t project(vec3_t point) {
    vec2_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z
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
            {
                SDL_Log("SDLK_ESCAPE\n");
                is_running = false;
            }
            break;
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
    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;


    // Loop all triangle faces of our mesh
    int n_faces = array_length(mesh.faces);
    for (int i = 0; i < n_faces; i++) {
        face_t mesh_face = mesh.faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1]; // Minus 1 because mesh vertices start from 1.
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        triangle_t projected_triangle;

        // Loop all three vertices of this current face and apply transformations
        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];

            transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            // Translate the vertex away from the camera
            transformed_vertex.z -= camera.position.z;

            // Project the current vertex
            vec2_t projected_point = project(transformed_vertex);

            // Scale and translate the projected points to the middle of the screen
            projected_point.x += (window_width / 2);
            projected_point.y += (window_height / 2);

            projected_triangle.points[j] = projected_point;
        }

        // Save the projected triangle in the array of triangles to render
        //triangles_to_render[i] = projected_triangle;
        array_push(triangles_to_render, projected_triangle);
    }
}

void render(void) {

    clear_color_buffer( 0xFF000000 );
    draw_grid();

    // Loop all projected triangles and render them
    uint32_t color = 0xFFFFFF00;
    int num_tris = array_length(triangles_to_render);
    for (int i = 0; i < num_tris; i++) {
        triangle_t triangle = triangles_to_render[i];
        draw_triangle(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x, triangle.points[1].y, triangle.points[2].x, triangle.points[2].y, color);
    }

    // Clear the array of triangles to render every frame loop
    array_free(triangles_to_render);

    render_color_buffer();
}

void free_resources(void) {
  free(color_buffer);
  array_free(mesh.vertices);
  array_free(mesh.faces);
}

int main(int argc, char *argv[])
{
    SDL_Log("Hello courses.pikuma.com\n");
    is_running = init_window();
    setup();
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
