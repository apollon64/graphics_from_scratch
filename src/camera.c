#include "camera.h"

camera_t camera = {
    .position = { 0, 0, 0 },
    .dir = { 0, 0, 1 }
};

camera_t camera_init( vec3_t position, float hang, float vang, float fov, float aspect, float zNear, float zFar)
{
    mat4_t proj = mat4_make_perspective(fov,aspect,zNear,zFar);

    camera_t camera = {
        .position = position,
        .dir = {0,0,1},
        .posh = hang,
        .posv = vang,
        .oldMouseX = 0,
        .oldMouseY = 0,
        .proj_mat = proj
    };
    return camera;
}

void camera_controls(camera_t *camera, bool keySpace, bool shiftKey, int moveX, int moveZ, int mousx, int mousy, int bstatus, float dt)
{
   (void)&keySpace;
   float f = dt;
   if (shiftKey) f *= 8;
   float vx = moveX * f; //(keyRight-keyLeft)*f; //D-A
   float vy = 0; //(keyForward-keyBackward)*f; //W-S
   float vz = moveZ * f;//vz = (keystatus[0x52]-keystatus[0x9d])*f; //KP0-RCtrl
   //mat4x4 mat = mat4x4.ang2mat(posh,posv); // Convert angles to rotation matrix

   mat4_t rx = mat4_make_rotation_x(-camera->posv);
   mat4_t ry = mat4_make_rotation_y(-camera->posh);
   mat4_t mat = mat4_mul_mat4(rx,ry);

   // find new position
   camera->position.x += mat.m[0][0] * vx + mat.m[1][0] * vy + mat.m[2][0] * vz;
   camera->position.y += mat.m[0][1] * vx + mat.m[1][1] * vy + mat.m[2][1] * vz;
   camera->position.z += mat.m[0][2] * vx + mat.m[1][2] * vy + mat.m[2][2] * vz;

   // mousespeed values are tweaked for 60 fps
   float mouseSpeedX = -(mousx - camera->oldMouseX) * 0.01;
   float mouseSpeedY = -(mousy - camera->oldMouseY) * 0.01;
   camera->oldMouseX = mousx;
   camera->oldMouseY = mousy;
   if(bstatus==1)
   {
      camera->posh -= mouseSpeedX;
      camera->posv -= mouseSpeedY;
   }

   vec3_t xd; vec3_t yd; vec3_t zd;
   xd.x = mat.m[0][0]; xd.y = mat.m[0][1]; xd.z = mat.m[0][2];
   yd.x = mat.m[1][0]; yd.y = mat.m[1][1]; yd.z = mat.m[1][2];
   zd.x = mat.m[2][0]; zd.y = mat.m[2][1]; zd.z = mat.m[2][2];

   camera->side = (vec3_t){xd.x, yd.x, zd.x};
   camera->up   = (vec3_t){xd.y, yd.y, zd.y};
   camera->dir  = (vec3_t){xd.z, yd.z, zd.z};

   camera->view_mat =
           (mat4_t) {{
               {+xd.x, +xd.y, +xd.z, -vec3_dot(xd, camera->position)},
               {+yd.x, +yd.y, +yd.z, -vec3_dot(yd, camera->position)},
               {+zd.x, +zd.y, +zd.z, -vec3_dot(zd, camera->position)},
               {0,       0,      0,     1                           }
           }};

}
