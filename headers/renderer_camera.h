/*
===========================================================================
File:		renderer_camera.h
Author: 	Clinton Freeman
Created on: Feb 19, 2011
===========================================================================
*/

#ifndef RENDERER_CAMERA_H_
#define RENDERER_CAMERA_H_

void camera_init();
void camera_rotateModelview();
void camera_translateModelview();

void camera_translate  (vec3_t);
void camera_getPosition(vec3_t);

#endif /* RENDERER_CAMERA_H_ */
