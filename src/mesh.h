#ifndef MESH_H
#define MESH_H

#include "vector.h"
#include "triangle.h"

#define N_CUBE_VERTICES 8
extern vec3_t cube_vertices[N_CUBE_VERTICES];

#define N_CUBE_FACES (6 * 2) // 6 cube faces, 2 triangles per face
extern face_t cube_faces[N_CUBE_FACES];


typedef struct {
    vec3_t p;
    vec3_t n;
    vec2_t uv;
} mesh_vertex_t;

typedef struct {
    vec3_t* vertices; // dynamic array of vertices
    vec3_t* normals; // dynamic array
    vec2_t* texcoords; // dynamic array of texture coordinates
    face_t* faces;    // dynamic array of faces
    mesh_vertex_t* vertpack; // pos and texcoord together
    int32_t* indices;

    vec3_t translation;
    vec3_t rotation;  // oiler angles
    vec3_t scale;
} mesh_t;

mesh_t load_cube_mesh_data(void);
mesh_t load_obj_file_data(const char* filename);
void free_mesh(mesh_t* mesh);

#endif
