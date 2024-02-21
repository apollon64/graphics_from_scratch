#ifndef CAMERA_H
#define CAMERA_H

#include "vecmath.h"
#include "matrix.h"

#include <stdbool.h>

typedef struct {
    vec3_t position;
    vec3_t side;
    vec3_t up;
    vec3_t dir;

    mat4_t proj_mat;
    mat4_t view_mat;
    float posv;
    float posh;
    float oldMouseX, oldMouseY;
} camera_t;

extern camera_t camera;

camera_t camera_init(vec3_t position, float hang, float vang, float fov, float aspect, float zNear, float zFar);
void camera_controls(camera_t *camera, bool keySpace, bool shiftKey, int moveX, int moveZ, int mousx, int mousy, int bstatus, float dt);

#endif
