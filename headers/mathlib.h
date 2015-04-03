/*
===========================================================================
File:		mathlib.h
Author: 	Clinton Freeman
Created on: Sep 30, 2010
===========================================================================
*/

#ifndef MATHLIB_H_
#define MATHLIB_H_

#include <math.h>

#define M_PI_DIV180 0.01745329251994329576

//Vector typedefs
typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

typedef struct
{
	vec3_t a, b, c;
	vec3_t normal;
}
triangle_t;

//Vector utilities
#define DotProduct(x,y) (x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
#define VectorSubtract(a,b,c) {c[0]=a[0]-b[0];c[1]=a[1]-b[1];c[2]=a[2]-b[2];}
#define VectorAdd(a,b,c) {c[0]=a[0]+b[0];c[1]=a[1]+b[1];c[2]=a[2]+b[2];}
#define VectorCopy(a,b) {b[0]=a[0];b[1]=a[1];b[2]=a[2];}
#define VectorScale(a,b,c) {c[0]=b*a[0];c[1]=b*a[1];c[2]=b*a[2];}
#define VectorClear(x) {x[0] = x[1] = x[2] = 0;}
#define	VectorInverse(x) {x[0]=-x[0];x[1]=-x[1];x[2]=-x[2];}

void   CrossProduct    (const vec3_t v1, const vec3_t v2, vec3_t cross);
vec_t  VectorLength    (const vec3_t v);
vec_t  VectorNormalize (const vec3_t in, vec3_t out);

void glmatrix_identity(float *m);
void glmatrix_multiply(float *ma, float *mb, float *out);
void glmatrix_print(float *m);

float sin_t(float x);
float cos_t(float x);

void normalFromTri(vec3_t tri[3], vec3_t normal);
eboolean spherePlaneIntersect(vec3_t sCenter, vec_t sRadius, vec3_t pPoint, vec3_t pNormal);
eboolean triangleAABB(vec3_t boxOrigin, vec3_t halfwidths, vec3_t triangle[3], vec3_t tnormal);

#endif /* MATHLIB_H_ */
