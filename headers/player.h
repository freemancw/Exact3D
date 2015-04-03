/*
===========================================================================
File:		player.h
Author: 	Clinton Freeman
Created on: Dec 12, 2010
===========================================================================
*/

#ifndef PLAYER_H_
#define PLAYER_H_

#define MAX_SPEED 5.0

#define FORWARD 0
#define STRAFE  1
#define UP      2

void player_init();
void player_rotateX(float);
void player_rotateY(float);
void player_rotateZ(float);
void player_translate(vec3_t);
void player_calcNewPos();
void player_revertPos(vec3_t);

void player_getPosition(vec3_t);
void player_getHalfwidths(vec3_t);
void player_getLerpPosition(vec3_t);
void player_calcLerpPos(float);

void  player_getViewAngles   (vec3_t);
vec_t player_getViewHeight	 ();
void  player_getForwardVec   (vec3_t);

void player_getForwardSpeed (float *);
void player_getStrafeSpeed  (float *);
void player_getUpSpeed      (float *);

void player_setForwardSpeed (float);
void player_setStrafeSpeed  (float);
void player_setUpSpeed      (float);

#endif /* PLAYER_H_ */
