#ifndef MESH_H
#define MESH_H

#include "vecmath.h"
#include "triangle.h"

typedef struct {
    vec3_t p;
    vec3_t n;
    vec2_t uv;
} mesh_vertex_t;

typedef struct {
    unsigned a;
    unsigned b;
    unsigned c;
    unsigned texcoord_a;
    unsigned texcoord_b;
    unsigned texcoord_c;
    unsigned normal_a;
    unsigned normal_b;
    unsigned normal_c;
} face_t;

typedef struct {
    vec3_t* vertices; // dynamic array of vertices
    vec3_t* normals; // dynamic array
    vec2_t* texcoords; // dynamic array of texture coordinates
    face_t* faces;    // dynamic array of faces
    mesh_vertex_t* vertpack; // pos and texcoord together
    int32_t* indices;
} mesh_t;

mesh_t *mesh_from_obj(const char* filename);
void free_all_meshes();

mesh_t load_obj_file_data(const char* filename);
void free_mesh(mesh_t* mesh);

#endif
