/*
===========================================================================
File:		renderer_extensions.c
Author: 	Clinton Freeman
Created on: Oct 6, 2010
===========================================================================
*/

#include "headers/SDL/SDL.h"
#include "headers/SDL/SDL_opengl.h"

#include "headers/common.h"

#include "headers/renderer_extensions.h"

void renderer_extensions_init()
{
	glAttachShader 			= ( void 	(APIENTRY * ) 	(GLuint, GLuint) )
									SDL_GL_GetProcAddress("glAttachShader");

	glBindAttribLocation 	= ( void 	(APIENTRY * ) 	(GLuint, GLuint, const GLchar *) )
									SDL_GL_GetProcAddress("glBindAttribLocation");

	glCompileShader 		= ( void 	(APIENTRY * ) 	(GLuint) )
									SDL_GL_GetProcAddress("glCompileShader");

	glCreateProgram 		= ( GLuint 	(APIENTRY * ) 	(void) )
									SDL_GL_GetProcAddress("glCreateProgram");

	glCreateShader 			= ( GLuint 	(APIENTRY * ) 	(GLenum) )
									SDL_GL_GetProcAddress("glCreateShader");

	glDeleteProgram 		= ( void 	(APIENTRY * ) 	(GLuint) )
									SDL_GL_GetProcAddress("glDeleteProgram");

	glDeleteShader 			= ( void 	(APIENTRY * ) 	(GLuint) )
									SDL_GL_GetProcAddress("glDeleteShader");

	glShaderSource 			= ( void 	(APIENTRY * ) 	(GLuint, GLsizei, const GLchar* *, const GLint *) )
									SDL_GL_GetProcAddress("glShaderSource");

	glLinkProgram 			= ( void 	(APIENTRY * ) 	(GLuint) )
									SDL_GL_GetProcAddress("glLinkProgram");

	glUseProgram 			= ( void 	(APIENTRY * ) 	(GLuint) )
									SDL_GL_GetProcAddress("glUseProgram");

	glGetUniformLocation 	= ( GLint 	(APIENTRY * ) 	(GLuint, const GLchar *) )
									SDL_GL_GetProcAddress("glGetUniformLocation");

	glUniform1f 			= ( void 	(APIENTRY * ) 	(GLint, GLfloat) )
									SDL_GL_GetProcAddress("glUniform1f");

	glUniform2f 			= ( void 	(APIENTRY * ) 	(GLint, GLfloat, GLfloat) )
									SDL_GL_GetProcAddress("glUniform2f");

	glUniform3f 			= ( void 	(APIENTRY * ) 	(GLint, GLfloat, GLfloat, GLfloat) )
									SDL_GL_GetProcAddress("glUniform3f");

	glUniform4f 			= ( void 	(APIENTRY * ) 	(GLint, GLfloat, GLfloat, GLfloat, GLfloat) )
									SDL_GL_GetProcAddress("glUniform4f");

	glUniform1i 			= ( void 	(APIENTRY * ) 	(GLint, GLint) )
									SDL_GL_GetProcAddress("glUniform1i");

	glGetProgramiv 			= ( void 	(APIENTRY * ) 	(GLuint, GLenum, GLint *) )
									SDL_GL_GetProcAddress("glGetProgramiv");

	glGetProgramInfoLog 	= ( void 	(APIENTRY * ) 	(GLuint, GLsizei, GLsizei *, GLchar *) )
									SDL_GL_GetProcAddress("glGetProgramInfoLog");

	glGetShaderiv 			= ( void 	(APIENTRY * ) 	(GLuint, GLenum, GLint *) )
									SDL_GL_GetProcAddress("glGetShaderiv");

	glGetShaderInfoLog 		= ( void 	(APIENTRY * ) 	(GLuint, GLsizei, GLsizei *, GLchar *) )
									SDL_GL_GetProcAddress("glGetShaderInfoLog");
}
