/*
===========================================================================
File:		renderer_shader.c
Author: 	Clinton Freeman
Created on: Oct 5, 2010
===========================================================================
*/

#include "headers/SDL/SDL_opengl.h"

#include "headers/common.h"
#include "headers/files.h"

#include "headers/renderer_extensions.h"
#include "headers/renderer_shaders.h"

static GLuint sceneProgram = 0;

/*
 * renderer_shaders_init
 * TODO: Error checking
 */
void renderer_shaders_init()
{
	GLuint 		vertShader, fragShader;
	char   		*vertShaderText, *fragShaderText;
	const char 	*vs, *fs;

	vertShaderText  = files_readTextFile("shaders/fog.vert");
	fragShaderText  = files_readTextFile("shaders/fog.frag");

	vertShader = glCreateShader(GL_VERTEX_SHADER);
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	vs = vertShaderText; fs = fragShaderText;

	glShaderSource(vertShader, 1, &vs, NULL);
	glShaderSource(fragShader, 1, &fs, NULL);

	free(vertShaderText); free(fragShaderText);

	glCompileShader(vertShader);
	glCompileShader(fragShader);

	sceneProgram = glCreateProgram();
	glAttachShader(sceneProgram, fragShader);
	glAttachShader(sceneProgram, vertShader);

	glLinkProgram(sceneProgram);
	glUseProgram(sceneProgram);
}

GLuint renderer_shader_getProgram() { return sceneProgram; }
