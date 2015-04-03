/*
===========================================================================
File:		player_main.c
Author: 	Clinton Freeman
Created on: Nov 10, 2010
===========================================================================
*/

#include "headers/common.h"
#include "headers/mathlib.h"

#include "headers/player.h"

typedef struct
{
	vec3_t halfwidths;
	vec3_t position;
	vec3_t velocity;
	vec3_t speed;
	vec3_t viewangles_deg;
	vec3_t viewangles_rad;
	vec_t  viewheight;
}
player_t;

static player_t player;
static vec3_t oldPosition, newPosition, lerpPosition;

/*
 * player_init
 */
void player_init()
{
	player.halfwidths[_X] = 16.0;
	player.halfwidths[_Y] = 16.0;
	player.halfwidths[_Z] = 28.0;

	player.viewheight = 16.0;

	player.position[_X] = player.position[_Y] = 0.0;
	player.position[_Z] = player.halfwidths[_Z] + 32.0;
	player.speed[FORWARD] = player.speed[STRAFE] = player.speed[UP] = 0.0;

	//New velocity stuff
	player.velocity[_X] = player.velocity[_Y] = player.velocity[_Z] = 0.0;

	player.viewangles_deg[_X] = player.viewangles_deg[_Y] = player.viewangles_deg[_Z] = 0.0;
	player_rotateX(90.0);
}

/*
 * player_calcLerpPos
 */
void player_calcLerpPos(float delta)
{
	if(delta == 0.0)
	{
		VectorCopy(oldPosition, lerpPosition);
		return;
	}

	VectorSubtract(newPosition, oldPosition, lerpPosition);
	VectorScale(lerpPosition, 1.0/(33.0/delta), lerpPosition);
	VectorAdd(oldPosition, lerpPosition, lerpPosition);
}

/*
 * player_rotateX
 * Contains special checks to simulate a "head", i.e. you cannot rotate your head
 * through your chest to see behind you.
 */
void player_rotateX(float deg)
{
	player.viewangles_deg[_X] += deg;

	if(player.viewangles_deg[_X] >= 90.0)
		player.viewangles_deg[_X] = 90.0;
	else if(player.viewangles_deg[_X] <= -90.0)
		player.viewangles_deg[_X] = -90.0;

	player.viewangles_rad[_X] = player.viewangles_deg[_X] * M_PI_DIV180;

	printf("%f\n%f\n", player.viewangles_deg[_X], deg);
}

/*
 * player_rotateY
 */
void player_rotateY(float deg)
{
	player.viewangles_deg[_Y] += deg;
	player.viewangles_rad[_Y] = player.viewangles_deg[_Y] * M_PI_DIV180;
}

/*
 * player_rotateZ
 */
void player_rotateZ(float deg)
{
	player.viewangles_deg[_Z] += deg;
	player.viewangles_rad[_Z] = player.viewangles_deg[_Z] * M_PI_DIV180;
}

/*
 * player_translate
 */
void player_translate(vec3_t delta)
{
	VectorAdd(player.position, delta, player.position);
}

/*
 * player_translateForward
 */
static void player_translateForward()
{
	vec3_t forward;

	player_getForwardVec(forward);

	VectorScale(forward, player.speed[FORWARD], forward);
	VectorAdd(player.position, forward, player.position);
}

/*
 * player_translateStrafe
 */
static void player_translateStrafe()
{
	float dx, dy;
	float rightRad;

	rightRad = (player.viewangles_deg[_Z]-90.0) * M_PI_DIV180;

	dx = -sin_t(rightRad)*player.speed[STRAFE];
	dy =  cos_t(rightRad)*player.speed[STRAFE];

	player.position[_X] += dx;
	player.position[_Y] += dy;
}

/*
 * player_translateUp
 */
static void player_translateUp()
{
	//float dz;

	//dz = player.velocity[_Z] +;


	float dz;

	dz = player.speed[UP];
	player.position[_Z] += dz;

}

/*
 * player_calcNewPos
 */
void player_calcNewPos()
{
	VectorCopy(newPosition, oldPosition);

	player_translateForward();
	player_translateStrafe();
	player_translateUp();

	//printf("X: %f, Y: %f, Z: %f\n", player.position[_X], player.position[_Y], player.position[_Z]);
	//printf("F: %f, S: %f, U: %f\n", player.speed[FORWARD], player.speed[STRAFE], player.speed[UP]);

	VectorCopy(player.position, newPosition);
}

/*
 * player_revertPos
 */
void player_revertPos(vec3_t normal)
{
	vec3_t velVec;
	vec3_t pushNormal;
	float velProj, playerZSpeed;

	//Create a velocity vector
	VectorSubtract(newPosition, oldPosition, velVec);

	//Project the velocity vector onto the surface normal
	velProj = DotProduct(velVec, normal);

	//Translate the player by the normal scaled by the projection
	VectorScale(normal, (-velProj*1.002), pushNormal);

	player_translate(pushNormal);
	player_getUpSpeed(&playerZSpeed);
	//playerZSpeed = velProj;
	//playerZSpeed = (1.0 - pushNormal[_Z]);
	//player_setUpSpeed(playerZSpeed);
	//Decrease down speed appropriately


	//Go back to "safe" position
	//VectorCopy(oldPosition, newPosition);
	//VectorCopy(newPosition, player.position);

	//VectorScale(normal, 2.0, pushNormal);
	//player.speed[FORWARD] = player.speed[STRAFE] = 0.0;

	//player_translate(pushNormal);
	VectorCopy(player.position, newPosition);
}

/* Simple getters/setters */

vec_t player_getViewHeight() { return player.viewheight; }

void player_getViewAngles   (vec3_t dst) { VectorCopy(player.viewangles_rad, dst); }
void player_getPosition     (vec3_t dst) { VectorCopy(player.position, dst);       }
void player_getLerpPosition (vec3_t dst) { VectorCopy(lerpPosition, dst);          }
void player_getHalfwidths   (vec3_t dst) { VectorCopy(player.halfwidths, dst);     }

void player_getForwardVec(vec3_t dst)
{
	vec3_t forward;
	float sinZ, cosZ, sinX, cosX;

	sinZ = sin_t(player.viewangles_rad[_Z]);
	cosZ = cos_t(player.viewangles_rad[_Z]);
	sinX = sin_t(player.viewangles_rad[_X]);
	cosX = cos_t(player.viewangles_rad[_X]);

	/*
	forward[_X] =  sinZ * cosX;
	forward[_Y] = -cosZ * cosX;
	forward[_Z] =  sinX;
	*/

	forward[_X] =  sinZ;
	forward[_Y] = -cosZ;
	forward[_Z] = 0.0;

	VectorCopy(forward, dst);
}

void player_getForwardSpeed (float *dst) { *dst = player.speed[FORWARD]; }
void player_getStrafeSpeed  (float *dst) { *dst = player.speed[STRAFE];  }
void player_getUpSpeed      (float *dst) { *dst = player.speed[UP];      }

void player_setForwardSpeed (float s) { player.speed[FORWARD] = s; }
void player_setStrafeSpeed  (float s) { player.speed[STRAFE]  = s; }
void player_setUpSpeed      (float s) { player.speed[UP]      = s; }
