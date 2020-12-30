#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include <math.h>
#include "func.h"
#include "display.h"
#include "vector.h"

bool is_running;
struct mouse_t{
	int x,y;
} mouse;

// Review of structs
typedef struct {
	vec3_t position;
	vec3_t rotation;
	float fov_angle;
} camera_t;

camera_t camera = {
  {0,0,-5.f},
	{0.3, -2.0, 0.0},
	0.78
};

double time;

#define N_POINTS (9 * 9 * 9)
vec3_t cube_points[N_POINTS];
vec2_t projected_points[N_POINTS];
float fov_factor = 640;

void setup(void) {
	time = 0.0;

	int point_count = 0;
// Start loading my array of vectors
// from -1 to 1 (in this 9x9x9 cube)

for (float x = 0; x < 9; x++) {
	for (float y = 0; y < 9; y++) {
		for (float z = 0; z < 9; z++) {
			float xx = -1.0f + 2.0f * x/9.f;
			float yy = -1.0f + 2.0f * y/9.f;
			float zz = -1.0f + 2.0f * z/9.f;
			vec3_t new_point = { .x = xx, .y = yy, .z = zz };
			cube_points[point_count++] = new_point;
		}
	}
}
}

vec2_t project(vec3_t point) {

  // BC/DE = AB/AD
	//P'x = Px/Pz
	point.z += camera.position.z;
	vec2_t projected_point = {
		.x = fov_factor * point.x/point.z ,
		.y = fov_factor * point.y/point.z
	};
	return projected_point;
}

void process_input(void) {
	SDL_Event event;
	SDL_PollEvent(&event);
	switch(event.type) {
		case SDL_QUIT:
			is_running = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				is_running = false;
			}
			break;
		case SDL_MOUSEMOTION:
		{
			mouse.x = event.motion.x;
			mouse.y = event.motion.y;
		}
		break;
	}
}

void update(void) {
  time += 0.01;

	for (int i = 0; i < N_POINTS; i++) {
		vec3_t point = cube_points[i];

		// Project the current point
		vec2_t projected_point = project(point);

		// Save the projected 2D vector in the array of projected points
		projected_points[i] = projected_point;
	}
}

void render(void) {

	clear_color_buffer( 0xFF000000 );
	draw_grid();
	draw_rect(mouse.x,mouse.y,50,50,packColor(255,0,255) );
	//circle(50,50,50);
	line(0,0,mouse.x,mouse.y, 0xFFFF0000);

	// Loop all projected points and render them
	for (int i = 0; i < N_POINTS; i++) {
		vec2_t projected_point = projected_points[i];
		draw_rect(
			projected_point.x + (window_width / 2),
			projected_point.y + (window_height / 2),
			4,
			4,
			0xFFFFFF00
		);
	}


	render_color_buffer();
}

int main(int argc, char *argv[])
{
	SDL_Log("Hello courses.pikuma.com\n");
	SDL_Log("sizeof int %I64u\n", sizeof(int) );

	is_running = init_window();

	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"sizeof int %I64u\n", sizeof(int) );

	setup();
	while (is_running) {
		process_input();
		update();
		render();
	}
	destroy_window();
	return EXIT_SUCCESS;
}
