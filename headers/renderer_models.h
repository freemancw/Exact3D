/*
===========================================================================
File:		renderer_models.h
Author: 	Clinton Freeman
Created on: Feb 19, 2011
===========================================================================
*/

#ifndef RENDERER_MODELS_H_
#define RENDERER_MODELS_H_

#define MAX_MODELS   128

typedef int modelHandle_t;

modelHandle_t renderer_model_loadASE(char *name, eboolean collidable, eboolean clamp);
void renderer_model_drawASE(modelHandle_t index);

#endif /* RENDERER_MODELS_H_ */
