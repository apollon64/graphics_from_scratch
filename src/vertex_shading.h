#ifndef VERTEX_SHADING_H
#define VERTEX_SHADING_H


#include "matrix.h"
#include "vecmath.h"
#include "mesh.h"
#include "texture.h"

#include <stdbool.h>
#include <stdint.h>

enum eCull_method {
    CULL_NONE,
    CULL_BACKFACE
};

enum eRender_method {
    RENDER_WIRE,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE,
    RENDER_TEXTURED,
    RENDER_TEXTURED_WIRE
};


typedef struct {
    vec4_t a,b;
} line_t;

typedef struct
{
    mat4_t model_matrix;
    mat4_t view_matrix;
    mat4_t projection_matrix;
    uint32_t color;
} uniforms_t;

typedef struct
{
    mesh_t mesh;
    texture_t* texture;
    uniforms_t uniforms;
    enum eRender_method drawmode;
    int polylist_begin;
    int polylist_end;
} draw_call_t;

// TODO some kind of getter system for these stretchy buffers
extern float z_near;
extern float z_far;
extern bool display_normals_enable;

typedef struct
{
    uint32_t num_triangles_issued;
    uint32_t num_triangles_culled;
    uint32_t num_cull_backface;
    uint32_t num_cull_zero_area;
    uint32_t num_cull_small_area;
    uint32_t num_cull_degenerate;
    uint32_t num_cull_near;
    uint32_t num_cull_far;
    uint32_t num_cull_xy;
    uint32_t num_clip_output_degen; // Clip resulted in non-tri.
} vertex_shading_stats;
extern vertex_shading_stats vss;

extern enum eCull_method cull_method;
extern enum eRender_method render_method;

triangle_t *get_triangles_to_render();
line_t* get_lines_to_render();
int getNumTris();
void freeTris();
void sort_triangles();
// Can be done on a model-by-model basis
// or that we accumulate all models into a vertex buffer
// and then blast through that
void vertexShading(mesh_t mesh, uniforms_t uniforms);
void vertexShading2(mesh_t mesh, mat4_t mvp);
void addDrawcall(vec3_t pos, vec3_t rot, enum eRender_method mode, mesh_t *mesh, texture_t *texture, uniforms_t uniforms);
void clearDrawcalls();
void pk_vertex_shading_step(int option);
draw_call_t * get_drawcall_list();

#endif
