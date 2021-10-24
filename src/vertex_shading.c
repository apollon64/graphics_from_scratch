#include "vertex_shading.h"

#include "clip.h"
#include "array.h" // stretchy buffer

#include "stretchy_buffer.h"
#include "func.h"

#include <stddef.h> // NULL
#include <math.h>
#include <assert.h>

#define CLIP_POS_X 1
#define CLIP_NEG_X 2
#define CLIP_POS_Y 4
#define CLIP_NEG_Y 8
#define CLIP_POS_Z 16
#define CLIP_NEG_Z 32

static triangle_t *triangles_to_render = NULL;
static line_t* lines_to_render = NULL;
static draw_call_t* drawcall_list = NULL;

triangle_t *get_triangles_to_render() { return triangles_to_render; }
line_t* get_lines_to_render() { return lines_to_render; }

float z_near = 0.01f;
float z_far = 200.0f;
bool display_normals_enable = false;
int num_culled;
int num_cull_backface;
int num_cull_zero_area;
int num_cull_small_area;
int num_cull_degenerate;
int num_not_culled;
int num_cull_near;
int num_cull_far;
int num_cull_xy;
int num_cull_few;
int num_cull_many;

int cull_left = 0;
int cull_right = 0;
int cull_bottom = 0;
int cull_top = 0;
int cull_near = 0;
int cull_far = 0;

//#define DYNAMIC_MEM_EACH_FRAME 1

int getNumTris()
{
#if defined(DYNAMIC_MEM_EACH_FRAME)
    return array_length(triangles_to_render);
#else
    return sb_count(triangles_to_render);
#endif
}

void freeTris()
{
#if defined(DYNAMIC_MEM_EACH_FRAME)
    array_free(triangles_to_render);
    array_free(lines_to_render);
    array_free(drawcall_list);
#endif
}

static void clearTris()
{
#if defined(DYNAMIC_MEM_EACH_FRAME)
    triangles_to_render = NULL;
    lines_to_render = NULL;
    drawcall_list = NULL;
#else
    // Set Stretchy Buffer size (not capacity) to zero
    if (triangles_to_render != NULL) stb__sbn(triangles_to_render)=0;
    if (lines_to_render != NULL) stb__sbn(lines_to_render)=0;
    if (drawcall_list != NULL) stb__sbn(drawcall_list)=0;
#endif
}

static void snap(float *v)
{
  *v = floorf( (*v) * 128.f)/128.f;
    //*v = floorf( (*v) * .5)/.5;
}

vec4_t to_screen_space(vec4_t v)
{
    v.x *= (pk_window_width() / 2.0f);
    v.y *= (pk_window_height() / 2.0f);

    // Invert screen y coordinate since our display goes from 0 to get_window_height()
    v.y *= -1;

    v.x += (pk_window_width() / 2.0f);
    v.y += (pk_window_height() / 2.0f);

    snap(&v.x);
    snap(&v.y);
    // If clipping is good, x,y should be within {0,xres} and {0,yres}
    return v;
}

static bool isBackface(vec2_t v0, vec2_t v1, vec2_t v2, float *area2)
{
    vec2_t v0v1 = vec2_sub(v1,v0);
    vec2_t v0v2 = vec2_sub(v2,v0);
    *area2 = fabsf((v0v1.x * v0v2.y) - (v0v1.y * v0v2.x));
    // If this is done after project and window coord transform where we may flip Y then handedness changes
    if (v0v2.x * v0v1.y < v0v2.y * v0v1.x) return 1; // change to < for right hand coords or >= for left... or was it other way round ...
    return 0;
}

static void addLineToRender(vec3_t normal, vec3_t center, mat4_t mvp_matrix)
{
  float line_length = 20.f / (.5f*pk_window_width());
  normal = vec3_mul(normal, line_length);
  vec4_t start = mat4_mul_vec4_project(mvp_matrix, vec4_from_vec3(center) );
  vec4_t end = mat4_mul_vec4_project(mvp_matrix, vec4_from_vec3( vec3_add(center, normal)) );
  if (start.w < 0 && end.w < 0) return; // TODO proper 3D line clipping
  line_t projected_line = {.a = to_screen_space(start), .b = to_screen_space(end) };
  array_push(lines_to_render, projected_line);
}

static void addTriangleToRender(triangle_t projected_triangle)
{
  // Save the projected triangle in the array of triangles to render
#if defined(DYNAMIC_MEM_EACH_FRAME)
    array_push(triangles_to_render, projected_triangle);
#else
  sb_push(triangles_to_render, projected_triangle);
#endif
}

void vertexShadingInit()
{
    clearTris();
    deleteDrawcalls();
}

void vertexShading2(mesh_t mesh, mat4_t mvp)
{
    num_culled = 0;
    num_cull_backface = 0;
    num_cull_zero_area = 0;
    num_cull_small_area = 0;
    num_cull_degenerate = 0;
    num_cull_near = 0;
    num_cull_far = 0;
    num_not_culled = 0;
    num_cull_xy = 0;
    num_cull_few = 0;
    num_cull_many = 0;

    cull_left = 0;
    cull_right = 0;
    cull_bottom = 0;
    cull_top = 0;
    cull_near = 0;
    cull_far = 0;

    int n_positions = array_length(mesh.vertpack);
    vec4_t* shaded_positions = NULL;
    uint8_t* clip_codes = NULL;
    clip_codes = array_hold(clip_codes, n_positions, sizeof(clip_codes[0]) );
    shaded_positions = array_hold(shaded_positions, n_positions, sizeof(shaded_positions[0]) );

    // Vertex Transform is simply multiply by MVP and divide by W (project)

    for (int i = 0; i < n_positions; i++)
    {
        // Transform our model space xyz to clipcoords
        vec4_t transformed_vertex = mat4_mul_vec4(mvp,  vec4_from_vec3(mesh.vertpack[i].p));

        // Dont divide by W yet, since we want to clip in clipcoords
        float w1 = transformed_vertex.w;

        // Bitmask of which planes vertex is on. 0 if inside all planes.
        uint8_t clip_code =
                (transformed_vertex.x <= -w1)<<0u |
                (transformed_vertex.x >= +w1)<<1u |
                (transformed_vertex.y <= -w1)<<2u |
                (transformed_vertex.y >= +w1)<<3u |
                (transformed_vertex.z <= -w1)<<4u |
                (transformed_vertex.z >= +w1)<<5u;

        clip_codes[i] = clip_code;
        shaded_positions[i] = transformed_vertex;
    }

    int n_elems = array_length(mesh.indices);

    // Triangle Assembly and Clipping
    for (int i = 0; i < n_elems; i+=3) {
        // Find the 3 indices in vertex buffers that map to a triangle vertices {i0,i1,i2}
        int idx0 = mesh.indices[i+0];
        int idx1 = mesh.indices[i+1];
        int idx2 = mesh.indices[i+2];

        // Find the combined clip by OR-ing all 3 vertex clips together
        U8 combined_clip = clip_codes[idx0] | clip_codes[idx1] | clip_codes[idx2];
        U8 combined_clip_and = clip_codes[idx0] & clip_codes[idx1] & clip_codes[idx2]; // bit only set if all 3 have it

        if (combined_clip)
        {
            // If all 3 verts are outside one clip plane, we can completely discard this triangle
            if (combined_clip_and & CLIP_POS_X) { cull_left++; continue;}
            if (combined_clip_and & CLIP_NEG_X) { cull_right++; continue;}
            if (combined_clip_and & CLIP_POS_Y) { cull_bottom++; continue;}
            if (combined_clip_and & CLIP_NEG_Y) { cull_top++; continue;}
            if (combined_clip_and & CLIP_POS_Z) { cull_near++; continue;}
            if (combined_clip_and & CLIP_NEG_Z) { cull_far++; continue;}
        }


        // Positions are transformed already
        vec4_t transformed_vertices[3] = {
            shaded_positions[idx0],
            shaded_positions[idx1],
            shaded_positions[idx2],
        };
        int num_zero_w = 0;
        for (int j = 0; j < 3; j++) {
            num_zero_w += transformed_vertices[j].w <= 0.f;
        }
        if (num_zero_w == 3)
        {
            num_cull_zero_area++;
            num_culled++;
            continue;
        }

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertpack[idx0].p;
        face_vertices[1] = mesh.vertpack[idx1].p;
        face_vertices[2] = mesh.vertpack[idx2].p;

        vec2_t face_texcoords[3];
        face_texcoords[0] = mesh.vertpack[idx0].uv;
        face_texcoords[1] = mesh.vertpack[idx1].uv;
        face_texcoords[2] = mesh.vertpack[idx2].uv;

        uint32_t face_colors[3];
        face_colors[0] = 0xFFFF0000;//vec3_to_uint32_t(mesh.colors[mesh_face.a ]);
        face_colors[1] = 0xFF00FF00;//vec3_to_uint32_t(mesh.colors[mesh_face.b ]);
        face_colors[2] = 0xFF0000FF;//vec3_to_uint32_t(mesh.colors[mesh_face.c ]);

        vec3_t face_normal = mesh.vertpack[idx0].n;

        vec3_t center = {0,0,0};
        center = vec3_add(center, face_vertices[0] );
        center = vec3_add(center, face_vertices[1] );
        center = vec3_add(center, face_vertices[2] );
        center = vec3_mul(center, 1.0f / 3.0f);


        if (combined_clip)
        {
            polygon_t polygon = create_polygon_from_triangle(
                transformed_vertices[0],
                transformed_vertices[1],
                transformed_vertices[2],
                face_texcoords[0],
                face_texcoords[1],
                face_texcoords[2]
            );

            clip_polygon2(&polygon);
            if (polygon.num_vertices < 3)
            {
                num_cull_few++;
                num_culled++;
                continue;
            }
            vec4_t transformed_and_clipped_vertices[3];

            for(int tri=0; tri<polygon.num_vertices - 2; tri++)
            {
                transformed_and_clipped_vertices[0] = polygon.vertices[0]; // Notice always first
                transformed_and_clipped_vertices[1] = polygon.vertices[tri+1];
                transformed_and_clipped_vertices[2] = polygon.vertices[tri+2];

                triangle_t projected_triangle;
                projected_triangle.z = 0.f;// Z sort
                for (int j=0; j<3; j++)
                {
                    vec4_t projected_point = vec4_project(transformed_and_clipped_vertices[j]);
                    projected_point = to_screen_space(projected_point);

                    projected_triangle.points[j].x = projected_point.x;
                    projected_triangle.points[j].y = projected_point.y;
                    projected_triangle.points[j].z = projected_point.z;
                    projected_triangle.points[j].w = projected_point.w;

                    projected_triangle.z = fmax( projected_triangle.z, projected_point.z );
                }

                float x0 = projected_triangle.points[0].x;
                float x1 = projected_triangle.points[1].x;
                float x2 = projected_triangle.points[2].x;
                float y0 = projected_triangle.points[0].y;
                float y1 = projected_triangle.points[1].y;
                float y2 = projected_triangle.points[2].y;

                if ((x0==x1 && y0==y1) || (x1==x2 && y1==y2) || (x2==x0 && y2==y0))
                {
                    num_cull_degenerate++;
                    //num_culled++; // Since we clipped, we may have more tris
                    continue;
                }

                projected_triangle.texcoords[0] = polygon.texcoords[0];
                projected_triangle.texcoords[1] = polygon.texcoords[tri+1];
                projected_triangle.texcoords[2] = polygon.texcoords[tri+2];

                vec2_t a = vec2_from_vec4( projected_triangle.points[0] );
                vec2_t b = vec2_from_vec4( projected_triangle.points[1] );
                vec2_t c = vec2_from_vec4( projected_triangle.points[2] );
                float area2;
                bool backfacing = !isBackface( a, b, c, &area2 );
                if (cull_method == CULL_BACKFACE && backfacing)
                {
                    num_cull_backface++;
                    num_culled++;
                    continue;
                }

                projected_triangle.area2 = area2;

                projected_triangle.center = center;
                projected_triangle.normal = face_normal;
                // projected_triangle.colors = face_colors[0]; // TODO lerp colors

                addTriangleToRender(projected_triangle);
            }


        } else {
            triangle_t projected_triangle;

            projected_triangle.z = 0.f; // Zsort
            for (int j=0; j<3; j++)
            {
                vec4_t projected_point = vec4_project(transformed_vertices[j]);
                projected_triangle.points[j] = to_screen_space(projected_point);
                projected_triangle.z = fmax( projected_triangle.z, projected_triangle.points[j].z );
            }

            projected_triangle.texcoords[0] = face_texcoords[0];
            projected_triangle.texcoords[1] = face_texcoords[1];
            projected_triangle.texcoords[2] = face_texcoords[2];

            vec2_t a = vec2_from_vec4( projected_triangle.points[0] );
            vec2_t b = vec2_from_vec4( projected_triangle.points[1] );
            vec2_t c = vec2_from_vec4( projected_triangle.points[2] );
            float area2;
            bool backfacing = !isBackface( a, b, c, &area2 );
            if (cull_method == CULL_BACKFACE && backfacing)
            {
                num_cull_backface++;
                num_culled++;
                continue;
            }
            projected_triangle.area2 = area2;
            assert(area2>0.0f);
            projected_triangle.center = center;
            projected_triangle.normal = face_normal;

            addTriangleToRender(projected_triangle);
            if (display_normals_enable)
            {
                addLineToRender(face_normal, center, mvp);
            }

        }

    } // n indices

    array_free(shaded_positions);
    array_free(clip_codes);
}

void vertexShading(mesh_t mesh, mat4_t model_matrix, mat4_t view_matrix, mat4_t projection_matrix)
{
    clearTris();
    // mvp = M V P
    mat4_t mvp = mat4_mul_mat4(projection_matrix, view_matrix);
    mvp = mat4_mul_mat4(mvp, model_matrix);

    num_culled = 0;
    num_cull_backface = 0;
    num_cull_zero_area = 0;
    num_cull_small_area = 0;
    num_cull_degenerate = 0;
    num_cull_near = 0;
    num_cull_far = 0;
    num_not_culled = 0;
    num_cull_xy = 0;
    num_cull_few = 0;
    num_cull_many = 0;

    // Loop all triangle faces of our mesh
    // It would be more efficient to extract all verts, texcoords, normals from mesh on startup to a mesh VertexBuffer
    // and then transform all vertices in a tight loop
    // and then draw them using an index buffer
    int n_faces = array_length(mesh.faces);
    for (int i = 0; i < n_faces; i++) {
        face_t mesh_face = mesh.faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a];
        face_vertices[1] = mesh.vertices[mesh_face.b];
        face_vertices[2] = mesh.vertices[mesh_face.c];

        vec2_t face_texcoords[3];
        face_texcoords[0] = mesh.texcoords[mesh_face.texcoord_a];
        face_texcoords[1] = mesh.texcoords[mesh_face.texcoord_b];
        face_texcoords[2] = mesh.texcoords[mesh_face.texcoord_c];

        uint32_t face_colors[3];
        face_colors[0] = 0xFFFF0000;//vec3_to_uint32_t(mesh.colors[mesh_face.a ]);
        face_colors[1] = 0xFF00FF00;//vec3_to_uint32_t(mesh.colors[mesh_face.b ]);
        face_colors[2] = 0xFF0000FF;//vec3_to_uint32_t(mesh.colors[mesh_face.c ]);

        vec3_t center = {0,0,0};
        center = vec3_add(center, face_vertices[0]);
        center = vec3_add(center, face_vertices[1]);
        center = vec3_add(center, face_vertices[2]);
        center = vec3_mul(center, 1.0f / 3.0f);


        vec4_t transformed_vertices[3];
        // Loop all three vertices of this current face and apply transformations
        for (int j = 0; j < 3; j++) {
            // Transform face by concatenated MVP matrix, then divide by W (projection)
            //vec4_t transformed_vertex = mat4_mul_vec4_project(mvp, vec4_from_vec3(face_vertices[j]) );
            // Multiply the world matrix by the original vector
            vec4_t transformed_vertex = mat4_mul_vec4(model_matrix,  vec4_from_vec3(face_vertices[j]));

            // Multiply the view matrix by the vector to transform the scene to camera space
            transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

            // Save transformed vertex in the array of transformed vertices
            transformed_vertices[j] = transformed_vertex;
        }

        // Save world space triangle
        polygon_t polygon = create_polygon_from_triangle(
            transformed_vertices[0],
            transformed_vertices[1],
            transformed_vertices[2],
            face_texcoords[0],
            face_texcoords[1],
            face_texcoords[2]
        );

        int num_zero_w = 0;
        int num_out_near = 0;
        int num_out_far = 0;

        for (int j = 0; j < 3; j++) {
            num_zero_w += transformed_vertices[j].w <= 0.f;
            num_out_near += transformed_vertices[j].z < z_near;
            num_out_far += transformed_vertices[j].z > z_far;
        }

        if (num_zero_w == 3)
        {
            num_cull_zero_area++;
            num_culled++;
            continue;
        }

        if (num_out_near == 3)
        {
            num_cull_near++;
            num_culled++;
            continue;
        }

        if (num_out_far == 3)
        {
            num_cull_far++;
            num_culled++;
            continue;
        }

        // Before transforming to screen space, find area in world space
        // Check backface culling
        vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]); /*   A   */
        vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]); /*  / \  */
        vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]); /* C---B */

        // Get the vector subtraction of B-A and C-A
        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);
        vec3_t transformed_normal = vec3_cross(vector_ab, vector_ac);

        // Scale and translate the projected points to the middle of the screen
        for (int j = 0; j < 3; j++) {
            transformed_vertices[j] = mat4_mul_vec4_project( projection_matrix, transformed_vertices[j] );
            transformed_vertices[j] = to_screen_space(transformed_vertices[j]);
        }



        vec3_t face_normal = vec3_cross(
                            vec3_sub(face_vertices[1],face_vertices[0]),
                            vec3_sub(face_vertices[2],face_vertices[0])
                        );
        vec3_normalize(&face_normal);

        // Find the vector between a point in the triangle and camera origin
        vec3_t origin = {0.f, 0.f, 0.f};
        vec3_t camera_ray = vec3_sub(origin, vector_a);
        float rayDotNormal = vec3_dot(camera_ray, transformed_normal);

        // Bypass the triangles looking away from camera
        float area2 = 0;
        bool backfacing = isBackface(
              (vec2_t) {transformed_vertices[0].x, transformed_vertices[0].y},
              (vec2_t) {transformed_vertices[1].x, transformed_vertices[1].y},
              (vec2_t) {transformed_vertices[2].x, transformed_vertices[2].y},
              &area2
                          );
        //float area2 = vec3_dot(transformed_normal, transformed_normal);
        if (cull_method == CULL_BACKFACE && !backfacing)
        {
            num_cull_backface++;
            num_culled++;
            continue;
        }
//        if ( area2 < 0.000001f)
//        {
//            // Think of a triangle that is 10 wide, 10 high. it has an area of 100 / 2 = 50
//            // Triangles can be surprisingly small yet contribute to image
//            num_cull_small_area++;
//            num_culled++;
//            continue;
//        }

//        bool front_facing = rayDotNormal > 0.0f;
//        //front_facing = !backfacing;
//        assert(backfacing == !front_facing);
//        if (cull_method == CULL_BACKFACE && !front_facing)
//        {
//            num_cull_backface++;
//            num_culled++;
//            continue;
//        }


          clip_polygon(&polygon); //, frustum_planes);
          if (polygon.num_vertices < 3)
          {
              num_cull_few++;
              num_culled++;
              continue;
          }

          if (polygon.num_vertices==3)
          {
              if (display_normals_enable)
              {
                  addLineToRender(face_normal, center, mvp);
              }
          }

          vec4_t transformed_and_clipped_vertices[3];

            for(int tri=0; tri<polygon.num_vertices - 2; tri++)
            {
              transformed_and_clipped_vertices[0] = polygon.vertices[0]; // Notice always first
              transformed_and_clipped_vertices[1] = polygon.vertices[tri+1];
              transformed_and_clipped_vertices[2] = polygon.vertices[tri+2];
              // Since we didnt do project yet, set W to one
              transformed_and_clipped_vertices[0].w = 1.0f;
              transformed_and_clipped_vertices[1].w = 1.0f;
              transformed_and_clipped_vertices[2].w = 1.0f;

              triangle_t projected_triangle;
              projected_triangle.z = 0.f;
              for (int j=0; j<3; j++)
              {
                vec4_t projected_point = mat4_mul_vec4_project( projection_matrix, transformed_and_clipped_vertices[j] );

                projected_point = to_screen_space(projected_point);
                // TODO add culling if all 2 xes or 2 ys same

                projected_triangle.points[j].x = projected_point.x;
                projected_triangle.points[j].y = projected_point.y;
                projected_triangle.points[j].z = projected_point.z;
                projected_triangle.points[j].w = projected_point.w;

                projected_triangle.z = fmax( projected_triangle.z, projected_point.z );

              }

              float x0 = projected_triangle.points[0].x;
              float x1 = projected_triangle.points[1].x;
              float x2 = projected_triangle.points[2].x;
              float y0 = projected_triangle.points[0].y;
              float y1 = projected_triangle.points[1].y;
              float y2 = projected_triangle.points[2].y;

              if ((x0==x1 && y0==y1) || (x1==x2 && y1==y2) || (x2==x0 && y2==y0))
              {
                  num_cull_degenerate++;
                  //num_culled++; // Since we clipped, we may have more tris
                  continue;
              }


              projected_triangle.texcoords[0] = polygon.texcoords[0];
              projected_triangle.texcoords[1] = polygon.texcoords[tri+1];
              projected_triangle.texcoords[2] = polygon.texcoords[tri+2];

              vec2_t vec1 = vec2_sub(
                        vec2_from_vec4( projected_triangle.points[1] ),
                        vec2_from_vec4( projected_triangle.points[2] ) );
              vec2_t vec2 = vec2_sub(
                        vec2_from_vec4( projected_triangle.points[0] ),
                        vec2_from_vec4( projected_triangle.points[2] ) );

              float area2 = fabsf((vec1.x * vec2.y) - (vec1.y * vec2.x));
              projected_triangle.area2 = area2;

              projected_triangle.center = center;
              projected_triangle.normal = face_normal;
              // projected_triangle.colors = face_colors[0]; // TODO lerp colors

              addTriangleToRender(projected_triangle);
            }
    } // nfaces

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

void sort_triangles()
{
    qsort(get_triangles_to_render(), getNumTris(), sizeof(triangle_t), cmpLess);
}

void addDrawcall(mesh_t mesh, texture_t* t, mat4_t mvp)
{
    draw_call_t dc = {.mesh = mesh, .texture = t, .mvp = mvp, .polylist_begin = -1, .polylist_end = -1};
    array_push(drawcall_list, dc);
}

void deleteDrawcalls()
{
    // see clearTris
    //array_free(drawcall_list);
}

void shadeDrawcalls()
{
    int num_draws = array_length(drawcall_list);
    for(int i=0; i<num_draws; i++)
    {
        int polys_before = getNumTris();
        vertexShading2( drawcall_list[i].mesh, drawcall_list[i].mvp );
        int polys_after = getNumTris();
        drawcall_list[i].polylist_begin = polys_before;
        drawcall_list[i].polylist_end = polys_after;
    }
}

draw_call_t *get_drawcall_list()
{
    return drawcall_list;
}
