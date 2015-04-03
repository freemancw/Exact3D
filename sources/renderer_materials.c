/*
===========================================================================
File:		renderer_img_texture.c
Author: 	Clinton Freeman
Created on: Oct 8, 2010
===========================================================================
*/

#include "headers/SDL/SDL_opengl.h"

#include "headers/common.h"
#include "headers/files.h"
#include "headers/mathlib.h"

#include "headers/renderer_materials.h"

typedef struct
{
	int		glTexID;
	char	name[128];
	vec3_t	ambient, diffuse, specular;
	float	shine, shineStrength, transparency;
	int		width, height, bpp;
}
material_t;

static material_t materialList[MAX_TEXTURES];
static materialHandle_t lastPtr = 0;

/*
 * renderer_img_createMaterial
 */
materialHandle_t renderer_img_createMaterial(char *name, vec3_t ambient, vec3_t diffuse, vec3_t specular,
		float shine, float shineStrength, float transparency, eboolean clamp)
{
	material_t *currentMat = &materialList[lastPtr];

	currentMat->shine 			= shine;
	currentMat->shineStrength 	= shineStrength;
	currentMat->transparency 	= transparency;

	strcpy(currentMat->name, name);

	VectorCopy(ambient,  currentMat->ambient);
	VectorCopy(diffuse,  currentMat->diffuse);
	VectorCopy(specular, currentMat->specular);

	renderer_img_loadTGA(name, &(currentMat->glTexID),
			&(currentMat->width), &(currentMat->height), &(currentMat->bpp));

	if(clamp)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	return lastPtr++;
}

int renderer_img_getMatGLID  (materialHandle_t i) { return materialList[i].glTexID; }
int renderer_img_getMatWidth (materialHandle_t i) { return materialList[i].width;   }
int renderer_img_getMatHeight(materialHandle_t i) { return materialList[i].height;  }
int renderer_img_getMatBpp   (materialHandle_t i) { return materialList[i].bpp;     }
