/*
===========================================================================
File:		renderer_extensions.h
Author: 	Clinton Freeman
Created on: Feb 19, 2011
===========================================================================
*/

#ifndef RENDERER_EXTENSIONS_H_
#define RENDERER_EXTENSIONS_H_

#include <GL/glext.h>

void renderer_extensions_init();

void 	(APIENTRYP glAttachShader) 			(GLuint, GLuint);
void 	(APIENTRYP glBindAttribLocation) 	(GLuint, GLuint, const GLchar *);
void 	(APIENTRYP glCompileShader) 		(GLuint);
GLuint 	(APIENTRYP glCreateProgram) 		(void);
GLuint 	(APIENTRYP glCreateShader) 			(GLenum);
void 	(APIENTRYP glDeleteProgram) 		(GLuint);
void 	(APIENTRYP glDeleteShader) 			(GLuint);
void 	(APIENTRYP glShaderSource) 			(GLuint, GLsizei, const GLchar* *, const GLint *);
void 	(APIENTRYP glLinkProgram) 			(GLuint);
void 	(APIENTRYP glUseProgram) 			(GLuint);
GLint 	(APIENTRYP glGetUniformLocation) 	(GLuint, const GLchar *);
void 	(APIENTRYP glUniform1f) 			(GLint, GLfloat);
void 	(APIENTRYP glUniform2f) 			(GLint, GLfloat, GLfloat);
void 	(APIENTRYP glUniform3f) 			(GLint, GLfloat, GLfloat, GLfloat);
void 	(APIENTRYP glUniform4f) 			(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
void 	(APIENTRYP glUniform1i) 			(GLint, GLint);
void 	(APIENTRYP glGetProgramiv) 			(GLuint, GLenum, GLint *);
void 	(APIENTRYP glGetProgramInfoLog) 	(GLuint, GLsizei, GLsizei *, GLchar *);
void 	(APIENTRYP glGetShaderiv) 			(GLuint, GLenum, GLint *);
void 	(APIENTRYP glGetShaderInfoLog) 		(GLuint, GLsizei, GLsizei *, GLchar *);

#endif /* RENDERER_EXTENSIONS_H_ */
