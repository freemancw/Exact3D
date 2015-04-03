/*
===========================================================================
File:		world_public.h
Author: 	Clinton Freeman
Created on: Dec 12, 2010
===========================================================================
*/

#ifndef WORLD_H_
#define WORLD_H_

//Called by main loop (system_main)
void world_init();
void world_update();
void world_lerpPositions(float dt);

//Used by model loading (renderer_mesh_ase)
void world_allocCollisionTris(int numTris);
void world_addCollisionTri(vec3_t verts[3]);

#endif /* WORLD_H_ */
