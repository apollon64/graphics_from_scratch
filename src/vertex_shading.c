#include "vertex_shading.h"

#include "clip.h"
#include "array.h" // stretchy buffer

#include "func.h"
#include "triangle.h"
#include "draw_triangle_torb.h"

#include <stddef.h> // NULL
#include <stdio.h>
#include <stdlib.h> // qsort
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

static vec4_t* shaded_positions = NULL;
static uint8_t* clip_codes = NULL;

triangle_t *get_triangles_to_render() { return triangles_to_render; }
line_t* get_lines_to_render() { return lines_to_render; }

float z_near = 0.5f;
float z_far = 400.0f;
bool display_normals_enable = false;

vertex_shading_stats vss;

enum eCull_method cull_method = 0;
enum eRender_method render_method = 0;

int getNumTris()
{
    return array_length(triangles_to_render);
}

void freeTris()
{
    array_free(triangles_to_render);
    array_free(lines_to_render);
    array_free(drawcall_list);

    array_free(shaded_positions);
    array_free(clip_codes);
}

static void clearTris()
{
    array_size_clear(triangles_to_render);
    array_size_clear(lines_to_render);
}

void clearDrawcalls()
{
    array_size_clear(drawcall_list);
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
    vec2_t v2v1 = vec2_sub(v1,v2);
    vec2_t v2v0 = vec2_sub(v0,v2);
    float _area2x = (v2v1.y * v2v0.x) - (v2v1.x * v2v0.y);
    *area2 = fabsf(_area2x);
    return _area2x <= 0.0f;
    //return v2v1.x * v2v0.y < v2v1.y * v2v0.x;
    /*
    vec2_t v0v1 = vec2_sub(v1,v0);
    vec2_t v0v2 = vec2_sub(v2,v0);
    *area2 = fabsf((v0v1.x * v0v2.y) - (v0v1.y * v0v2.x));
    // If this is done after project and window coord transform where we may flip Y then handedness changes
    if (v0v2.x * v0v1.y < v0v2.y * v0v1.x) return 1; // change to < for right hand coords or >= for left... or was it other way round ...
    */
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

void vertexShading2(mesh_t mesh, mat4_t mvp)
{
    int n_positions = array_length(mesh.vertpack);

    clip_codes = array_hold(clip_codes, n_positions, sizeof(clip_codes[0]) );
    shaded_positions = array_hold(shaded_positions, n_positions, sizeof(shaded_positions[0]) );

    // Vertex Transform is simply multiply by MVP and divide by W (project)

    for (int i = 0; i < n_positions; i++)
    {
        // Transform our model space xyz to clipcoords
        vec4_t clipspace_vertex = mat4_mul_vec4(mvp,  vec4_from_vec3(mesh.vertpack[i].p));

        // Dont divide by W yet, since we want to clip in clipcoords
        float w1 = clipspace_vertex.w;

        // Bitmask of which planes vertex is on. 0 if inside all planes.
        uint8_t clip_code =
            (clipspace_vertex.x <= -w1)<<0u |
            (clipspace_vertex.x >= +w1)<<1u |
            (clipspace_vertex.y <= -w1)<<2u |
            (clipspace_vertex.y >= +w1)<<3u |
            (clipspace_vertex.z <= -w1)<<4u |
            (clipspace_vertex.z >= +w1)<<5u;

        uint8_t clip_code2 =
            (( clipspace_vertex.x + w1)<0) <<0u |
            ((-clipspace_vertex.x + w1)<0) <<1u |
            (( clipspace_vertex.y + w1)<0) <<2u |
            ((-clipspace_vertex.y + w1)<0) <<3u |
            (( clipspace_vertex.z + w1)<0) <<4u |
            ((-clipspace_vertex.z + w1)<0) <<5u;
        //assert(clip_code == clip_code2);
        clip_code = clip_code2;

        clip_codes[i] = clip_code;
        shaded_positions[i] = clipspace_vertex;
    }

    int n_indices = array_length(mesh.indices);
    vss.num_triangles_issued += n_indices / 3;

    // Triangle Assembly and Clipping
    for (int i = 0; i < n_indices; i+=3 /*step one triangle*/) {
        // Find the 3 indices in vertex buffers that map to a triangle vertices {i0,i1,i2}
        int idx0 = mesh.indices[i+0];
        int idx1 = mesh.indices[i+1];
        int idx2 = mesh.indices[i+2];

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
            ++vss.num_cull_zero_area;
            ++vss.num_triangles_culled;
            continue;
        }

        // Find the combined clip by OR-ing all 3 vertex clips together
        U8 clip0 = clip_codes[idx0];
        U8 clip1 = clip_codes[idx1];
        U8 clip2 = clip_codes[idx2];
        U8 combined_clip = clip0 | clip1 | clip2;
        U8 combined_clip_and = clip_codes[idx0] & clip_codes[idx1] & clip_codes[idx2]; // bit only set if all 3 have it


        if (combined_clip_and)
        {
            ++vss.num_triangles_culled;
            // If all 3 verts are outside one clip plane, we can completely discard this triangle
            if (combined_clip_and & CLIP_POS_X) { ++vss.num_cull_xy; continue;}
            else if (combined_clip_and & CLIP_NEG_X) { ++vss.num_cull_xy; continue;}
            else if (combined_clip_and & CLIP_POS_Y) { ++vss.num_cull_xy; continue;}
            else if (combined_clip_and & CLIP_NEG_Y) { ++vss.num_cull_xy; continue;}
            else if (combined_clip_and & CLIP_POS_Z) { ++vss.num_cull_far; continue;}
            else if (combined_clip_and & CLIP_NEG_Z) { ++vss.num_cull_near; continue;}
            //assert(0);
        }

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertpack[idx0].p;
        face_vertices[1] = mesh.vertpack[idx1].p;
        face_vertices[2] = mesh.vertpack[idx2].p;

        vec2_t face_texcoords[3];
        face_texcoords[0] = mesh.vertpack[idx0].uv;
        face_texcoords[1] = mesh.vertpack[idx1].uv;
        face_texcoords[2] = mesh.vertpack[idx2].uv;

        //uint32_t face_colors[3];
        //face_colors[0] = 0xFFFF0000;//vec3_to_uint32_t(mesh.colors[mesh_face.a ]);
        //face_colors[1] = 0xFF00FF00;//vec3_to_uint32_t(mesh.colors[mesh_face.b ]);
        //face_colors[2] = 0xFF0000FF;//vec3_to_uint32_t(mesh.colors[mesh_face.c ]);

        vec3_t face_normal = mesh.vertpack[idx0].n;

        vec3_t center = {0,0,0};
        center = vec3_add(center, face_vertices[0] );
        center = vec3_add(center, face_vertices[1] );
        center = vec3_add(center, face_vertices[2] );
        center = vec3_mul(center, 1.0f / 3.0f);


        if (combined_clip /*zz yy xx */ )
        {
            polygon_t polygon = create_polygon_from_triangle(
                transformed_vertices[0],
                transformed_vertices[1],
                transformed_vertices[2],
                face_texcoords[0],
                face_texcoords[1],
                face_texcoords[2]
                );

            clip_polygon(&polygon, CLIP_CLIP_SPACE /*use clip space*/);
            if (polygon.num_vertices < 3)
            {
                ++vss.num_clip_output_degen;
                ++vss.num_triangles_culled;
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
                    vec4_t ndc_coord = vec4_project(transformed_and_clipped_vertices[j]);
                    vec4_t screen_coord = to_screen_space(ndc_coord);

                    projected_triangle.points[j].x = screen_coord.x;
                    projected_triangle.points[j].y = screen_coord.y;
                    projected_triangle.points[j].z = screen_coord.z;
                    projected_triangle.points[j].w = screen_coord.w;

                    projected_triangle.z = fmax( projected_triangle.z, screen_coord.z );
                }

                float x0 = projected_triangle.points[0].x;
                float x1 = projected_triangle.points[1].x;
                float x2 = projected_triangle.points[2].x;
                float y0 = projected_triangle.points[0].y;
                float y1 = projected_triangle.points[1].y;
                float y2 = projected_triangle.points[2].y;

                if ((x0==x1 && y0==y1) || (x1==x2 && y1==y2) || (x2==x0 && y2==y0))
                {
                    ++vss.num_cull_degenerate;
                    //++vss.num_triangles_culled;; // Since we clipped, we may have more tris
                    continue;
                }

                projected_triangle.texcoords[0] = polygon.texcoords[0];
                projected_triangle.texcoords[1] = polygon.texcoords[tri+1];
                projected_triangle.texcoords[2] = polygon.texcoords[tri+2];

                vec2_t a = vec2_from_vec4( projected_triangle.points[0] );
                vec2_t b = vec2_from_vec4( projected_triangle.points[1] );
                vec2_t c = vec2_from_vec4( projected_triangle.points[2] );
                float area2;
                bool backfacing = isBackface( a, b, c, &area2 );
                if (cull_method == CULL_BACKFACE && backfacing)
                {
                    vss.num_cull_backface++;
                    ++vss.num_triangles_culled;
                    continue;
                }
                projected_triangle.area2 = area2;
                //projected_triangle.center = center;
                //projected_triangle.normal = face_normal; // i removed it
                // projected_triangle.colors = face_colors[0]; // TODO lerp colors

                array_push(triangles_to_render, projected_triangle);
            }

        } else {
            triangle_t projected_triangle;

            projected_triangle.z = 0.f; // Zsort
            for (int j=0; j<3; j++)
            {
                vec4_t ndc_coord = vec4_project(transformed_vertices[j]);
                projected_triangle.points[j] = to_screen_space(ndc_coord);
                projected_triangle.z = fmax( projected_triangle.z, projected_triangle.points[j].z );
            }

            projected_triangle.texcoords[0] = face_texcoords[0];
            projected_triangle.texcoords[1] = face_texcoords[1];
            projected_triangle.texcoords[2] = face_texcoords[2];

            vec2_t a = vec2_from_vec4( projected_triangle.points[0] );
            vec2_t b = vec2_from_vec4( projected_triangle.points[1] );
            vec2_t c = vec2_from_vec4( projected_triangle.points[2] );
            float area2;
            bool backfacing = isBackface( a, b, c, &area2 );
            if (cull_method == CULL_BACKFACE && backfacing)
            {
                ++vss.num_cull_backface;
                ++vss.num_triangles_culled;
                continue;
            }
            projected_triangle.area2 = area2;
            assert(area2>0.0f);
            //projected_triangle.center = center;
            //projected_triangle.normal = face_normal;

            array_push(triangles_to_render, projected_triangle);
            if (display_normals_enable)
            {
                addLineToRender(face_normal, center, mvp);
            }

        }

    } // n indices

    array_size_clear(shaded_positions);
    array_size_clear(clip_codes);
}

void vertexShading(mesh_t mesh, uniforms_t uniforms)
{
    // mvp = M V P
    mat4_t mvp = mat4_mul_mat4(uniforms.projection_matrix, uniforms.view_matrix);
    mvp = mat4_mul_mat4(mvp, uniforms.model_matrix);
    mat4_t modelview_matrix = mat4_mul_mat4(uniforms.view_matrix, uniforms.model_matrix);

    // Loop all triangle faces of our mesh
    // It would be more efficient to extract all verts, texcoords, normals from mesh on startup to a mesh VertexBuffer
    // and then transform all vertices in a tight loop
    // and then draw them using an index buffer
    int n_faces = array_length(mesh.faces);
    vss.num_triangles_issued += n_faces;
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

        vec4_t worldspace_vertices[3];
        vec4_t clipspace_vertices[3];
        vec4_t ndc_vertices[3];
        vec4_t screen_vertices[3];
        // Loop all three vertices of this current face and apply transformations
        for (int j = 0; j < 3; j++) {
            // Transform face by concatenated MVP matrix, then divide by W (projection)
            // Multiply the world matrix by the original vector
            vec4_t model_vertex = vec4_from_vec3(face_vertices[j]);
            // Multiply the view matrix by the vector to transform the scene to camera space
            worldspace_vertices[j] = mat4_mul_vec4(modelview_matrix, model_vertex);

            // Save transformed vertex in the array of transformed vertices
            clipspace_vertices[j] = mat4_mul_vec4(mvp, model_vertex);
            ndc_vertices[j] = vec4_project( clipspace_vertices[j] );

            // Scale and translate the projected points to the middle of the screen
            screen_vertices[j] = to_screen_space(ndc_vertices[j]);
        }

        int num_zero_w = 0;
        int num_out_near = 0;
        int num_out_far = 0;
        int num_out_xmin = 0;
        int num_out_xmax = 0;
        int num_out_ymin = 0;
        int num_out_ymax = 0;

        for (int j = 0; j < 3; j++) {
            num_zero_w += ndc_vertices[j].w <= 0.f;

            // Out near far can also be done on NDC by checking outside unit cube.
            //num_out_near += worldspace_vertices[j].z < z_near;
            //num_out_far += worldspace_vertices[j].z > z_far;
            num_out_near += ndc_vertices[j].z < -1.0f;
            num_out_far += ndc_vertices[j].z > +1.0f;

            num_out_xmin += ndc_vertices[j].x < -1.0f;
            num_out_xmax += ndc_vertices[j].x > +1.0f;
            num_out_ymin += ndc_vertices[j].y < -1.0f;
            num_out_ymax += ndc_vertices[j].y > +1.0f;
        }

        if (num_zero_w == 3)
        {
            ++vss.num_cull_zero_area;
            ++vss.num_triangles_culled;
            continue;
        }
        else if (num_out_near == 3)
        {
            ++vss.num_cull_near;
            ++vss.num_triangles_culled;
            continue;
        }
        else if (num_out_far == 3)
        {
            ++vss.num_cull_far;
            ++vss.num_triangles_culled;
            continue;
        }
        else if (num_out_xmin == 3 ||
                 num_out_xmax == 3 ||
                 num_out_ymin == 3 ||
                 num_out_ymax == 3
                 )
        {
            ++vss.num_cull_xy;
            ++vss.num_triangles_culled;
            continue;
        }

        // Bypass the triangles looking away from camera
        float area2;
        bool backfacing = isBackface(
            (vec2_t) {screen_vertices[0].x, screen_vertices[0].y},
            (vec2_t) {screen_vertices[1].x, screen_vertices[1].y},
            (vec2_t) {screen_vertices[2].x, screen_vertices[2].y},
            &area2
            );
        if (cull_method == CULL_BACKFACE && backfacing)
        {
            ++vss.num_cull_backface;
            ++vss.num_triangles_culled;
            continue;
        }
       // if ( area2 < 0.000001f)
       // {
       //     // Think of a triangle that is 10 wide, 10 high. it has an area of 100 / 2 = 50
       //     // Triangles can be surprisingly small yet contribute to image
       //     ++vss.num_cull_small_area;
       //     ++vss.num_triangles_culled;
       //     continue;
       // }

        // Save world space triangle
        polygon_t polygon = create_polygon_from_triangle(
            clipspace_vertices[0],
            clipspace_vertices[1],
            clipspace_vertices[2],
            face_texcoords[0],
            face_texcoords[1],
            face_texcoords[2]
            );
        clip_polygon(&polygon, CLIP_CLIP_SPACE /*use clip space*/);
        if (polygon.num_vertices < 3)
        {
            ++vss.num_clip_output_degen;
            ++vss.num_triangles_culled;
            continue;
        }

        vec4_t transformed_and_clipped_vertices[3];

        for(int tri=0; tri<polygon.num_vertices - 2; tri++)
        {
            transformed_and_clipped_vertices[0] = polygon.vertices[0]; // Notice always first
            transformed_and_clipped_vertices[1] = polygon.vertices[tri+1];
            transformed_and_clipped_vertices[2] = polygon.vertices[tri+2];
            // Since we didnt do project yet, set W to one
            //transformed_and_clipped_vertices[0].w = 1.0f;
            //transformed_and_clipped_vertices[1].w = 1.0f;
            //transformed_and_clipped_vertices[2].w = 1.0f;

            triangle_t projected_triangle;
            projected_triangle.z = 0.f;
            for (int j=0; j<3; j++)
            {
                vec4_t projected_point = vec4_project( transformed_and_clipped_vertices[j] );
                //vec4_t projected_point = mat4_mul_vec4_project( uniforms.projection_matrix, transformed_and_clipped_vertices[j] );
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
                ++vss.num_cull_degenerate;
                //++vss.num_triangles_culled; // Since we clipped, we may have more tris
                continue;
            }

            projected_triangle.texcoords[0] = polygon.texcoords[0];
            projected_triangle.texcoords[1] = polygon.texcoords[tri+1];
            projected_triangle.texcoords[2] = polygon.texcoords[tri+2];

            // Do backface check again, because it gives potentially new area
            bool backfacing = isBackface(
                (vec2_t) {projected_triangle.points[0].x, projected_triangle.points[0].y},
                (vec2_t) {projected_triangle.points[1].x, projected_triangle.points[1].y},
                (vec2_t) {projected_triangle.points[2].x, projected_triangle.points[2].y},
                &projected_triangle.area2
                );
            assert(!backfacing);

            array_push(triangles_to_render, projected_triangle);
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

void addDrawcall(vec3_t pos, vec3_t rot, enum eRender_method mode, mesh_t* mesh, texture_t* t, uniforms_t uniforms)
{
    // Create scale, rotation, and translation matrices that will be used to multiply the mesh vertices
    vec3_t scale = {1,1,1};
    mat4_t scale_matrix = mat4_make_scale(scale.x, scale.y, scale.z);
    mat4_t translation_matrix = mat4_make_translation(pos.x,pos.y,pos.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(rot.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(rot.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(rot.z);

    // Create a World Matrix combining scale, rotation, and translation matrices
    mat4_t model_matrix = mat4_identity();

    // Order matters: First scale, then rotate, then translate. [T]*[R]*[S]*v
    model_matrix = mat4_mul_mat4(scale_matrix, model_matrix);
    model_matrix = mat4_mul_mat4(rotation_matrix_x, model_matrix);
    model_matrix = mat4_mul_mat4(rotation_matrix_y, model_matrix);
    model_matrix = mat4_mul_mat4(rotation_matrix_z, model_matrix);
    model_matrix = mat4_mul_mat4(translation_matrix, model_matrix);

    uniforms.model_matrix = model_matrix;
    draw_call_t dc = {.mesh = *mesh, .drawmode=mode, .texture = t, .uniforms = uniforms, .polylist_begin = -1, .polylist_end = -1};
    array_push(drawcall_list, dc);
}

// Perform vertex shading on all draw calls in the drawcall list
// This function is called by the main loop
// The option parameter is used to determine which vertex shading function to use
// 0 = vertexShading
// 1 = vertexShading2
void pk_vertex_shading_step(int option)
{
    vss.num_triangles_culled = 0;
    vss.num_triangles_issued = 0;
    vss.num_cull_backface = 0;
    vss.num_cull_zero_area = 0;
    vss.num_cull_small_area = 0;
    vss.num_cull_degenerate = 0;
    vss.num_cull_near = 0;
    vss.num_cull_far = 0;
    vss.num_cull_xy = 0;
    vss.num_clip_output_degen = 0;

    clearTris();
    int num_draws = array_length(drawcall_list);
    for(int i=0; i<num_draws; i++)
    {
        int polys_before = getNumTris();

        uniforms_t uniforms = drawcall_list[i].uniforms;
        if (option==1) {
            mat4_t mvp = mat4_mul_mat4(uniforms.projection_matrix, uniforms.view_matrix);
            mvp = mat4_mul_mat4(mvp, uniforms.model_matrix);
            vertexShading2( drawcall_list[i].mesh, mvp );
        }
        else
            vertexShading( drawcall_list[i].mesh, drawcall_list[i].uniforms );

        int polys_after = getNumTris();
        drawcall_list[i].polylist_begin = polys_before;
        drawcall_list[i].polylist_end = polys_after;
    }
}

draw_call_t *get_drawcall_list()
{
    return drawcall_list;
}
