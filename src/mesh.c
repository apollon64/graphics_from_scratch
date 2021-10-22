#include "mesh.h"

#include <stddef.h> // NULL
#include <stdio.h>
#include <stdbool.h>
#include <string.h> // strncmp

#include "array.h"
#include "misc.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef struct { mesh_vertex_t key; int n; } key_vert_tex_t;
key_vert_tex_t* hashmap = NULL;

static mesh_t init_mesh()
{
    mesh_t mesh = {
        .vertices = NULL,
        .texcoords = NULL,
        .faces = NULL,
        .indices = NULL,
        .translation = { 0, 0, 0 },
        .rotation = { 0, 0, 0 },
        .scale = { 1.0, 1.0, 1.0 },
    };
    return mesh;
}

static faster_mesh_t init_faster_mesh()
{
    faster_mesh_t mesh = {
        .vertices = NULL,
        .texcoords = NULL,
        .indices = NULL,
        .translation = { 0, 0, 0 },
        .rotation = { 0, 0, 0 },
        .scale = { 1.0, 1.0, 1.0 },
    };
    return mesh;
}

vec3_t cube_vertices[N_CUBE_VERTICES] = {
    { .x = -1, .y = -1, .z = -1 }, // 1
    { .x = -1, .y =  1, .z = -1 }, // 2
    { .x =  1, .y =  1, .z = -1 }, // 3
    { .x =  1, .y = -1, .z = -1 }, // 4
    { .x =  1, .y =  1, .z =  1 }, // 5
    { .x =  1, .y = -1, .z =  1 }, // 6
    { .x = -1, .y =  1, .z =  1 }, // 7
    { .x = -1, .y = -1, .z =  1 }  // 8
};

face_t cube_faces[N_CUBE_FACES] = {
    // front
    { .a = 1, .b = 2, .c = 3 },
    { .a = 1, .b = 3, .c = 4 },
    // right
    { .a = 4, .b = 3, .c = 5 },
    { .a = 4, .b = 5, .c = 6 },
    // back
    { .a = 6, .b = 5, .c = 7 },
    { .a = 6, .b = 7, .c = 8 },
    // left
    { .a = 8, .b = 7, .c = 2 },
    { .a = 8, .b = 2, .c = 1 },
    // top
    { .a = 2, .b = 7, .c = 5 },
    { .a = 2, .b = 5, .c = 3 },
    // bottom
    { .a = 6, .b = 8, .c = 1 },
    { .a = 6, .b = 1, .c = 4 }
};

mesh_t load_cube_mesh_data(void) {
    mesh_t mesh = init_mesh();
    for (int i = 0; i < N_CUBE_VERTICES; i++) {
        array_push(mesh.vertices, cube_vertices[i]);
    }
    for (int i = 0; i < N_CUBE_FACES; i++) {
        array_push(mesh.faces, cube_faces[i]);
    }
    return mesh;
}

int min(int a, int b) { return a < b ? a : b; }

bool equal_vec3(vec3_t a, vec3_t b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool equal_vec2(vec2_t a, vec2_t b)
{
    return a.x == b.x && a.y == b.y;
}

bool equal_verts(mesh_vertex_t a, mesh_vertex_t b)
{
    return equal_vec3(a.p, b.p) && equal_vec2(a.uv, b.uv);
}

static void deduplicate1(face_t face, mesh_t *mesh)
{
    // For each vertex in a face
    // create a new vertex with all attributes
    // (position, normal, texturecoord) combined
    for (int i=0; i<3; i++)
    {
        mesh_vertex_t new_vertex;
        new_vertex.p = mesh->vertices[
                i == 0 ? face.a :
                i == 1 ? face.b :
                         face.c];

        new_vertex.uv = mesh->texcoords[
                i == 0 ? face.texcoord_a :
                i == 1 ? face.texcoord_b :
                         face.texcoord_c];


        int found_idx = 0;
        bool found_vertex = false;
        // It is far too much to loop over all vertices for each index added
        int num_indices = array_length(mesh->indices);
        int limit = min(64, num_indices );
        for(int n=0; n<limit; n++)
        {
            int old_index = mesh->indices[num_indices-1 - n];
            assert(old_index <= array_length(mesh->vertpack) ); // Cannot lookup something that doesnt exist
            if ( equal_verts(mesh->vertpack[old_index], new_vertex) )
            {
                found_vertex = true;
                found_idx = old_index;
                break;
            }
        }

        //assert( array_length(mesh->vertpack) < ((1<<16)-1) );
        assert( array_length(mesh->vertpack) < ((1u<<32u)-1u) );

        if (found_vertex)
        {
            array_push(mesh->indices, found_idx);
        }
        else
        {
            // Not seen before, add.
            int index = array_length( mesh->vertpack );
            array_push(mesh->vertpack, new_vertex);
            array_push(mesh->indices, index);
        }


    } // end for each vertex in face
}


static void deduplicate2(face_t face, mesh_t *mesh)
{
    // For each vertex in a face
    // create a new vertex with all attributes
    // (position, normal, texturecoord) combined
    for (int i=0; i<3; i++)
    {
        mesh_vertex_t new_vertex;
        new_vertex.p = mesh->vertices[
                i == 0 ? face.a :
                i == 1 ? face.b :
                         face.c];

        new_vertex.uv = mesh->texcoords[
                i == 0 ? face.texcoord_a :
                i == 1 ? face.texcoord_b :
                         face.texcoord_c];

        mesh_vertex_t mykey = { new_vertex.p, new_vertex.uv };
        key_vert_tex_t find = hmgets(hashmap, mykey);
        if ( !equal_verts(find.key, new_vertex) )
        {
            // Not seen before, add.
            int index = array_length( mesh->vertpack );
            array_push(mesh->vertpack, new_vertex);
            array_push(mesh->indices, index);

            key_vert_tex_t s = { mykey, index };
            hmputs(hashmap, s);

        } else {
            int found_idx = find.n;
            array_push(mesh->indices, found_idx);
        }


    } // end for each vertex in face
}

mesh_t load_obj_file_data(const char* filename) {
    FILE* file;
    file = fopen(filename, "r");

    mesh_t mesh = init_mesh();
    if (!file)
    {
        fprintf(stderr, "could not open file %s", filename);
        return mesh; // null mesh
    }
    char line[1024];

    while (fgets(line, 1024, file)) {
        // Vertex information
        if (strncmp(line, "v ", (size_t)2) == 0) {
            vec3_t vertex;
            sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
            array_push(mesh.vertices, vertex);
        }
        // Texture information
        if (strncmp(line, "vt ", (size_t)3) == 0) {
            vec2_t texcoord;
            sscanf(line, "vt %f %f", &texcoord.x, &texcoord.y);
            array_push(mesh.texcoords, texcoord);
        }

        // Face information
        if (strncmp(line, "f ", (size_t)2) == 0) {
            int vertex_indices[3];
            int texture_indices[3];
            int normal_indices[3];
            sscanf(
                line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vertex_indices[0], &texture_indices[0], &normal_indices[0],
                &vertex_indices[1], &texture_indices[1], &normal_indices[1],
                &vertex_indices[2], &texture_indices[2], &normal_indices[2]
            );
            face_t face = {
                .a = vertex_indices[0] - 1,
                .b = vertex_indices[1] - 1,
                .c = vertex_indices[2] - 1,
                .texcoord_a = texture_indices[0] - 1,
                .texcoord_b = texture_indices[1] - 1,
                .texcoord_c = texture_indices[2] - 1,
            };
            deduplicate1(face, &mesh);
            array_push(mesh.faces, face);
        }
    }

    int vertex_len = array_length(mesh.vertices);
    int texcoord_len = array_length(mesh.texcoords);
    int faces_len = array_length(mesh.faces);
    // An obj mesh may be optimized by finding vertices that are same
    fprintf(stdout, "loaded mesh, verts:%d, texcoords:%d, faces:%d \n", vertex_len, texcoord_len, faces_len);
    fclose(file);

    printf("the element buffer:%d and vertpack: %d\n", array_length( mesh.indices ), array_length( mesh.vertpack ) );
    hmfree(hashmap);
    assert(hashmap == NULL);
    return mesh;
}

void free_mesh(mesh_t* mesh)
{
    array_free(mesh->vertices);
    array_free(mesh->texcoords);
    array_free(mesh->faces);

    array_free(mesh->indices);
    array_free(mesh->vertpack);
}
