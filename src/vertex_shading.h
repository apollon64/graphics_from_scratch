#ifndef VERTEX_SHADING_H
#define VERTEX_SHADING_H


#include "matrix.h"
#include "vector.h"
#include "mesh.h"
#include "texture.h"

#include <stdbool.h>

typedef struct {
    vec4_t a,b;
} line_t;

typedef struct
{
    mesh_t mesh;
    texture_t* texture;
    mat4_t mvp;
    int polylist_begin;
    int polylist_end;
} draw_call_t;

triangle_t *get_triangles_to_render();
line_t* get_lines_to_render();
int getNumTris();
void freeTris();
void sort_triangles();

// TODO some kind of getter system for these stretchy buffers
extern float z_near;
extern float z_far;
extern bool display_normals_enable;

extern int num_culled;
extern int num_cull_backface;
extern int num_cull_zero_area;
extern int num_cull_small_area;
extern int num_cull_degenerate;
extern int num_not_culled;
extern int num_cull_near;
extern int num_cull_far;
extern int num_cull_xy;
extern int num_cull_few;
extern int num_cull_many;
extern int cull_left;
extern int cull_right;
extern int cull_bottom;
extern int cull_top;
extern int cull_near;
extern int cull_far;

enum eCull_method {
    CULL_NONE,
    CULL_BACKFACE
};

enum eRender_method {
    RENDER_WIRE,
    RENDER_WIRE_VERTEX,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE,
    RENDER_TEXTURED,
    RENDER_TEXTURED_WIRE
};

enum eCull_method cull_method;
enum eRender_method render_method;


// Can be done on a model-by-model basis
// or that we accumulate all models into a vertex buffer
// and then blast through that
void vertexShading(mesh_t mesh, mat4_t model_matrix, mat4_t view_matrix, mat4_t projection_matrix);
void vertexShadingInit();
void vertexShading2(mesh_t mesh, mat4_t mvp);
void addDrawcall(mesh_t mesh, texture_t *texture, mat4_t mvp);
void deleteDrawcalls();
void shadeDrawcalls();
draw_call_t * get_drawcall_list();

#endif
