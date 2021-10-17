#include "clip.h"

#include <math.h> // sin cos
#include <assert.h>
#include "func.h" // lerp
#include <stdio.h>

//static int sgn(float v) { return v >= 0.0f ? 1 : 0; }

static plane_t frustum_planes[6];

plane_t *get_frustum_planes()
{
    frustum_planes;
}

void init_frustum_planes(float fov_x, float fov_y, float z_near, float z_far)//, plane_t* frustum_planes)
{
    float cos_half_fov_x = cosf(fov_x / 2.f);
    float sin_half_fov_x = sinf(fov_x / 2.f);
    float cos_half_fov_y = cosf(fov_y / 2.f);
    float sin_half_fov_y = sinf(fov_y / 2.f);

    vec3_t origin = {0, 0, 0};

    frustum_planes[LEFT_FRUSTUM_PLANE].point = origin;
    frustum_planes[LEFT_FRUSTUM_PLANE].normal.x = cos_half_fov_x;
    frustum_planes[LEFT_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[LEFT_FRUSTUM_PLANE].normal.z = sin_half_fov_x;
    frustum_planes[RIGHT_FRUSTUM_PLANE].point = origin;
    frustum_planes[RIGHT_FRUSTUM_PLANE].normal.x = -cos_half_fov_x;
    frustum_planes[RIGHT_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[RIGHT_FRUSTUM_PLANE].normal.z = sin_half_fov_x;
    frustum_planes[TOP_FRUSTUM_PLANE].point = origin;
    frustum_planes[TOP_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[TOP_FRUSTUM_PLANE].normal.y = -cos_half_fov_y;
    frustum_planes[TOP_FRUSTUM_PLANE].normal.z = sin_half_fov_y;
    frustum_planes[BOTTOM_FRUSTUM_PLANE].point = origin;
    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.y = cos_half_fov_y;
    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.z = sin_half_fov_y;
    frustum_planes[NEAR_FRUSTUM_PLANE].point = origin;
    frustum_planes[NEAR_FRUSTUM_PLANE].point.z = z_near;
    frustum_planes[NEAR_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[NEAR_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[NEAR_FRUSTUM_PLANE].normal.z = 1;
    frustum_planes[FAR_FRUSTUM_PLANE].point = origin;
    frustum_planes[FAR_FRUSTUM_PLANE].point.z = z_far;
    frustum_planes[FAR_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[FAR_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[FAR_FRUSTUM_PLANE].normal.z = -1;
}

void clip_polygon_against_plane(polygon_t* polygon, int plane)  // plane_t* frustum_planes
{

  if (polygon->num_vertices <3) return;
  vec3_t plane_point = frustum_planes[plane].point;
  vec3_t plane_normal = frustum_planes[plane].normal;

  // Declare a static array of inside vertices that will be part of the final polygon returned via parameter
  vec3_t inside_vertices[MAX_NUM_POLY_VERTICES];
  vec2_t inside_texcoords[MAX_NUM_POLY_VERTICES];
  int num_inside_vertices = 0;

  // Start the current vertex with the first polygon vertex, and the previous with the last polygon vertex
  vec3_t* current_vertex = &polygon->vertices[0];
  vec3_t* previous_vertex = &polygon->vertices[polygon->num_vertices - 1];

  vec2_t* current_texcoord = &polygon->texcoords[0];
  vec2_t* previous_texcoord = &polygon->texcoords[polygon->num_vertices - 1];

  // Calculate the dot product of the current and previous vertex
  float current_dot = 0;
  float previous_dot = vec3_dot(vec3_sub(*previous_vertex, plane_point), plane_normal);

  // Loop all the polygon vertices while the current is different than the last one
  while (current_vertex != &polygon->vertices[polygon->num_vertices]) {
      current_dot = vec3_dot(vec3_sub(*current_vertex, plane_point), plane_normal);

      // If we changed from inside to outside or from outside to inside
      if (current_dot * previous_dot < 0) {
          // Find the interpolation factor t
          float t = previous_dot / (previous_dot - current_dot);
          // Calculate the intersection point I = Q1 + t(Q2-Q1)
//          vec3_t intersection_point = vec3_clone(current_vertex);              // I =        Qc
//          intersection_point = vec3_sub(intersection_point, *previous_vertex); // I =       (Qc-Qp)
//          intersection_point = vec3_mul(intersection_point, t);                // I =      t(Qc-Qp)
//          intersection_point = vec3_add(intersection_point, *previous_vertex); // I = Qp + t(Qc-Qp)

          // LERP is equivalent to finding intersection using vector math
          vec3_t intersection_point =
          {
                .x = lerp( previous_vertex->x, current_vertex->x, t),
                .y = lerp( previous_vertex->y, current_vertex->y, t),
                .z = lerp( previous_vertex->z, current_vertex->z, t),
          };

          vec2_t interpolated_tc =
          {
                .x = lerp( previous_texcoord->x, current_texcoord->x, t),
                .y = lerp( previous_texcoord->y, current_texcoord->y, t)
          };

          // Insert the intersection point to the list of "inside vertices"
          inside_vertices[num_inside_vertices] = intersection_point;
          inside_texcoords[num_inside_vertices] = interpolated_tc;
          num_inside_vertices++;
      }

      // Current vertex is inside the plane
      if (current_dot > 0) {
          // Insert the current vertex to the list of "inside vertices"
          inside_vertices[num_inside_vertices] = *current_vertex;
          inside_texcoords[num_inside_vertices] = *current_texcoord;
          num_inside_vertices++;
      }

      // Move to the next vertex
      previous_dot = current_dot;
      previous_vertex = current_vertex;
      previous_texcoord = current_texcoord;
      current_vertex++;
      current_texcoord++;
  }

  // At the end, copy the list of inside vertices into the destination polygon (out parameter)
  for (int i = 0; i < num_inside_vertices; i++) {
      polygon->vertices[i] = (inside_vertices[i]);
      polygon->texcoords[i] = (inside_texcoords[i]);
  }
  polygon->num_vertices = num_inside_vertices;

}

void clip_polygon(polygon_t* polygon/*, plane_t* frustum_planes*/) {
    clip_polygon_against_plane(polygon, LEFT_FRUSTUM_PLANE);//, frustum_planes);
    clip_polygon_against_plane(polygon, RIGHT_FRUSTUM_PLANE);//, frustum_planes);
    clip_polygon_against_plane(polygon, BOTTOM_FRUSTUM_PLANE);//, frustum_planes);
    clip_polygon_against_plane(polygon, TOP_FRUSTUM_PLANE);//, frustum_planes);
    clip_polygon_against_plane(polygon, NEAR_FRUSTUM_PLANE);//, frustum_planes);
    clip_polygon_against_plane(polygon, FAR_FRUSTUM_PLANE);//, frustum_planes);
}

polygon_t create_polygon_from_triangle(vec3_t v0, vec3_t v1, vec3_t v2,
                                           vec2_t tc0, vec2_t tc1, vec2_t tc2) {
    polygon_t p;
    p.vertices[0] = v0;
    p.vertices[1] = v1;
    p.vertices[2] = v2;

    p.texcoords[0] = tc0;
    p.texcoords[1] = tc1;
    p.texcoords[2] = tc2;

    p.num_vertices = 3;
    return p;
}
