#ifndef MESH_H
#define MESH_H

#include "vector.h"
#include "triangle.h"

#define N_CUBE_VERTICES 8
extern vec3_t cube_vertices[N_CUBE_VERTICES];

#define N_CUBE_FACES (6 * 2) // 6 cube faces, 2 triangles per face
extern face_t cube_faces[N_CUBE_FACES];

typedef struct {
    vec3_t* vertices; // dynamic array of vertices
    vec2_t* texcoords; // dynamic array of texture coordinates
    face_t* faces;    // dynamic array of faces
    vec3_t translation;
    vec3_t rotation;  // oiler angles
    vec3_t scale;
} mesh_t;

void load_cube_mesh_data(void);
void load_obj_file_data(const char* filename);

extern mesh_t mesh; // GLOBAL

#endif
