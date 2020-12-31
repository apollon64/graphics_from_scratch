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

// Review of structs
typedef struct {
    vec3_t position;
    vec3_t rotation;
    float fov_angle;
} camera_t;

triangle_t triangles_to_render[N_MESH_FACES];

camera_t camera = {
    {0,0,-5.f},
    {0.3, -2.0, 0.0},
    0.78
};
vec3_t cube_rotation = { .x = 0, .y = 0, .z = 0 };
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

    time += 0.01;
    cube_rotation.x += 0.01;
    cube_rotation.y += 0.01;
    cube_rotation.z += 0.01;

    // Loop all triangle faces of our mesh
    for (int i = 0; i < N_MESH_FACES; i++) {
        face_t mesh_face = mesh_faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh_vertices[mesh_face.a - 1]; // Minus 1 because mesh vertices start from 1.
        face_vertices[1] = mesh_vertices[mesh_face.b - 1];
        face_vertices[2] = mesh_vertices[mesh_face.c - 1];

        triangle_t projected_triangle;

        // Loop all three vertices of this current face and apply transformations
        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];

            transformed_vertex = vec3_rotate_x(transformed_vertex, cube_rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, cube_rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, cube_rotation.z);

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
        triangles_to_render[i] = projected_triangle;
    }
}

void drawline(bool type, int x0, int y0, int x1, int y1,uint32_t color)
{
  if (type) draw_line_dda(x0,y0,x1,y1,color); else draw_line(x0,y0,x1,y1,color);
}

void render(void) {

    clear_color_buffer( 0xFF000000 );
    draw_grid();
    if (mouse.left)
      draw_rect(mouse.x-25,mouse.y-25,50,50,packColor(255,0,255) );

    draw_rect(mouse.x-100,mouse.y-100,50,50,packColor(255,255,255) );

    if (mouse.right)
      draw_rect(mouse.x+50,mouse.y+50,50,50,packColor(0,0,255) );
    //circle(50,50,50);
    drawline(mouse.left,100,200,200,50, mouse.left ? 0xFFFF0000 : 0xFF0000FF);

    // Loop all projected triangles and render them
    uint32_t color = 0xFFFFFF00;
    for (int i = 0; i < N_MESH_FACES; i++) {
        triangle_t triangle = triangles_to_render[i];
        int h = 4;
        draw_rect(triangle.points[0].x-h, triangle.points[0].y-h, 8, 8, color);
        draw_rect(triangle.points[1].x-h, triangle.points[1].y-h, 8, 8, color);
        draw_rect(triangle.points[2].x-h, triangle.points[2].y-h, 8, 8, color);

        draw_triangle(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x, triangle.points[1].y, triangle.points[2].x, triangle.points[2].y, color);
    }

    render_color_buffer();
}

int main(int argc, char *argv[])
{
    SDL_Log("Hello courses.pikuma.com\n");

    is_running = init_window();

    //SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Not an error" );

    setup();
    while (is_running) {
        process_input();
        update();
        render();
    }
    destroy_window();
    SDL_Log("App closed. is_running=%d", is_running );
    return EXIT_SUCCESS;
}
