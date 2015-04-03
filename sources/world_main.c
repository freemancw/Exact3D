/*
===========================================================================
File:		world_main.c
Author: 	Clinton Freeman
Created on: Oct 5, 2010
===========================================================================
*/

#include "headers/common.h"
#include "headers/mathlib.h"

#include "headers/world_public.h"
#include "headers/player.h"

typedef struct
{
	vec3_t verts[3];
	vec3_t normal;
}
collisionTri_t;

static collisionTri_t *collisionList;

static int collisionListPtr;
static int collisionListSize;

/*
 * world_init
 */
void world_init()
{
	player_init();
}

/*
 * world_update
 */
void world_update()
{
	int i;
	vec3_t boxOrigin, halfwidths;
	vec_t upspeed;

	//player_getUpSpeed(&upspeed);
	//player_setUpSpeed(upspeed - 0.1);

	player_calcNewPos();

	player_getPosition(boxOrigin);
	player_getHalfwidths(halfwidths);

	for(i = 0; i < collisionListSize; i++)
	{
		if(triangleAABB(boxOrigin, halfwidths, collisionList[i].verts, collisionList[i].normal))
		{
			player_revertPos(collisionList[i].normal);
		}
	}

}

/*
 * world_lerpPositions
 * Linearly interpolate any positions that are being used for drawing.
 * The interpolated position is used to eliminate stuttering between
 * physics/collision updates.
 */
void world_lerpPositions(float dt)
{
	player_calcLerpPos(dt);
}

/*
 * world_allocCollisionTris
 */
void world_allocCollisionTris(int numTris)
{
	collisionListSize += numTris;
	collisionList = (collisionTri_t *)realloc(collisionList, sizeof(collisionTri_t) * collisionListSize);
}

/*
 * world_addCollisionTri
 */
void world_addCollisionTri(vec3_t verts[3])
{
	int i;
	vec3_t normal;

	for(i = 0; i < 3; i++)
		VectorCopy(verts[i], collisionList[collisionListPtr].verts[i]);

	normalFromTri(verts, normal);
	VectorCopy(normal, collisionList[collisionListPtr].normal);

	collisionListPtr++;
}
