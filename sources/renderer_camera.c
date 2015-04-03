/*
===========================================================================
File:		renderer_camera.c
Author: 	Clinton Freeman
Created on: Nov 10, 2010
===========================================================================
*/

#include "headers/SDL/SDL_opengl.h"

#include "headers/common.h"
#include "headers/mathlib.h"

#include "headers/renderer_camera.h"
#include "headers/player.h"

typedef struct
{
	vec3_t	position;
	vec3_t	rotation_deg;
	vec3_t	rotation_rad;
}
camera_t;

static camera_t camera;

static GLfloat xRotMatrix[16], yRotMatrix[16], zRotMatrix[16], translateMatrix[16];

static const GLfloat flipMatrix[16] =
{1.0, 0.0,  0.0, 0.0,
 0.0, 0.0, -1.0, 0.0,
 0.0, 1.0,  0.0, 0.0,
 0.0, 0.0,  0.0, 1.0};

/*
 * camera_setupProjection
 */
static void camera_setupProjection()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, ((GLfloat)WINDOW_WIDTH)/((GLfloat)WINDOW_HEIGHT), 0.5, 2048.0);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

/*
 * camera_init
 */
void camera_init()
{
	glmatrix_identity(xRotMatrix);
	glmatrix_identity(yRotMatrix);
	glmatrix_identity(zRotMatrix);
	glmatrix_identity(translateMatrix);

	//Only needs to be done once, for now.
	//May need to be modified later if we add zooming, etc.
	camera_setupProjection();
}

/*
 * camera_rotateX
 */
void camera_rotateX(float deg)
{
	camera.rotation_deg[_X] += deg;

	if(camera.rotation_deg[_X] >= 90.0)
		camera.rotation_deg[_X] = 90.0;
	else if(camera.rotation_deg[_X] <= -90.0)
		camera.rotation_deg[_X] = -90.0;

	camera.rotation_rad[_X] = camera.rotation_deg[_X] * M_PI_DIV180;
}

/*
 * camera_rotateY
 */
void camera_rotateY(float deg)
{
	camera.rotation_deg[_Y] += deg;
	camera.rotation_rad[_Y] = camera.rotation_deg[_Y] * M_PI_DIV180;
}

/*
 * camera_rotateZ
 */
void camera_rotateZ(float deg)
{
	camera.rotation_deg[_Z] += deg;
	camera.rotation_rad[_Z] = camera.rotation_deg[_Z] * M_PI_DIV180;
}

/*
 * camera_rotateModelview
 */
void camera_rotateModelview()
{
	static float sinX, sinZ, cosX, cosZ;

	player_getViewAngles(camera.rotation_rad);

	sinX = sin_t(camera.rotation_rad[_X]);
	sinZ = sin_t(camera.rotation_rad[_Z]);
	cosX = cos_t(camera.rotation_rad[_X]);
	cosZ = cos_t(camera.rotation_rad[_Z]);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	xRotMatrix[5] =  cosX;
	xRotMatrix[6] =  sinX;
	xRotMatrix[9] = -sinX;
	xRotMatrix[10] = cosX;

	zRotMatrix[0] =  cosZ;
	zRotMatrix[1] = -sinZ;
	zRotMatrix[4] =  sinZ;
	zRotMatrix[5] =  cosZ;

	glMultMatrixf(flipMatrix);
	glMultMatrixf(xRotMatrix);
	glMultMatrixf(zRotMatrix);
}

/*
 * camera_translateModelview
 */
void camera_translateModelview()
{
	static vec3_t position;

	//Grab the position
	player_getLerpPosition(position);
	//Adjust to the "eyes"
	position[_Z] += player_getViewHeight();
	//Move our camera
	camera_translate(position);

	glMatrixMode(GL_MODELVIEW);
	glMultMatrixf(translateMatrix);
}

/*
 * camera_translate
 */
void camera_translate(vec3_t direction)
{
	translateMatrix[12] = -direction[_X];
	translateMatrix[13] = -direction[_Y];
	translateMatrix[14] = -direction[_Z];
}

void camera_getPosition(vec3_t dst)      { VectorCopy(camera.position, dst);      }
void camera_setPosition(vec3_t position) { VectorCopy(position, camera.position); }
