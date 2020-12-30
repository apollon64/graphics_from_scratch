#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
  float x,y;
} vec2_t;

typedef struct {
  float x,y,z;
} vec3_t;

typedef struct {
  float x,y,z,w;
} vec4_t;

typedef struct {
  float m00,m10;
  float m01,m11;
} mat2_t;
/*
typedef struct {
  float m[16];
} mat4_t;
*/
typedef struct {
  float m00,m10,m20,m30;
  float m01,m11,m21,m31;
  float m02,m12,m22,m32;
  float m03,m13,m23,m33;
} mat4_t;

vec3_t vec3_rotate_x(vec3_t v, float a);
vec3_t vec3_rotate_y(vec3_t v, float a);
vec3_t vec3_rotate_z(vec3_t v, float a);
/*
mat4 scaleMatrix(x,y,z)
// axis x,y,z
// yaw pitch roll
mat4 rotationMatrix(vec3 axis)
mat4 translationMatrix(vec3 translation)

multiply3x3(mat3 a, mat3 b)
multiply4x4(mat4 a, mat4 b)
transformVec3Mat3(mat3 m, vec3 v)
transformVec4Mat4(mat4 m, vec4 v)
sine,
cosine,
tangent
SOH CAH TOA
*/

//[ ca -sa] |x|
//[ sa ca ] |y|

/*
mat2_t createRotation2D(float a)
{
  float ca = cos(a);
  float sa = sin(a);
  a.m00 = ca;
  a.m10 = -sa;
  a.m01 = sa;
  a.m11 = ca;
}

vec2 rotate2D(vec2 v, float b)
{
  // the angle additional form for sine
  x' = v.x*cos(b) - v.y*sin(b);
  y' = v.x*sin(b) + v.y*cos(b);
}
*/
#endif
