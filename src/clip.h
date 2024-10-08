#pragma once

#include "vecmath.h"

#define MAX_NUM_POLY_VERTICES 11
enum {
    LEFT_FRUSTUM_PLANE,
    RIGHT_FRUSTUM_PLANE,
    TOP_FRUSTUM_PLANE,
    BOTTOM_FRUSTUM_PLANE,
    NEAR_FRUSTUM_PLANE,
    FAR_FRUSTUM_PLANE
};

typedef enum {
    CLIP_WORLD_SPACE,
    CLIP_CLIP_SPACE,
} clip_space;

typedef struct  {
    vec3_t point;
    vec3_t normal;
} plane_t;

typedef struct {
    vec4_t vertices[MAX_NUM_POLY_VERTICES];
    vec2_t texcoords[MAX_NUM_POLY_VERTICES];
    int num_vertices;
} polygon_t;

void init_frustum_planes(float fov_x, float fov_y, float z_near, float z_far);//, plane_t* frustum_planes);
void clip_polygon_against_plane(polygon_t* polygon, clip_space space, int plane/*, plane_t* frustum_planes*/);
void clip_polygon_against_plane2(polygon_t* polygon, float a, float b, float c, float d);
void clip_polygon(polygon_t* polygon, clip_space space);
void clip_polygon2(polygon_t* polygon);
polygon_t create_polygon_from_triangle(vec4_t v0, vec4_t v1, vec4_t v2, vec2_t tc0, vec2_t tc1, vec2_t tc2);
