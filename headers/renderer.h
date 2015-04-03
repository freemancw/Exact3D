/*
===========================================================================
File:		renderer.h
Author: 	Clinton Freeman
Created on: Sep 29, 2010
===========================================================================
*/

/*
 * Notes about texture/model loading
 * -the application does not need to hold onto all of the image data after
 * uploading a texture to opengl
 * -instead we can just have a simple structure that links attributes to
 * the opengl-given texture ID
 * -this means that we will statically allocate enough space for some
 * pre-determined number of texture ID - attribute mappings
 * -we will dynamically allocate space for the image data during loading
 * and release it afterward
 * -model data is somewhat trickier
 * -we do not know how many meshes a particular ASE file will contain ahead
 * of time
 * -nor do we know how many verticies etc. it will have
 * -we could use a similar storage method where we statically allocate a given
 * amount of space to each model, with a maximum amount of verts, etc.
 * -however, the world will likely have far more data than things like weapon
 * models etc.
 * -we could classify types of models (world, weapon, player, etc.) and have
 * specially tailored maximum values for each
 * -one thing we could also do would be to only statically hang on to world data
 * (since we have to do things like collision detection with it), and for other
 * models just 1)dynamically allocate space, 2)compile a display list, 3)maintain
 * a mapping between the display list and the object itself, 4)release memory
 * afterward
 */

#ifndef RENDERER_H_
#define RENDERER_H_

#define MAX_TEXTURES 512
#define MAX_MODELS   128

//void renderer_init();
//void renderer_drawFrame();

void renderer_2D_init();
void renderer_draw2D();

int renderer_img_createMaterial(char *name, vec3_t ambient, vec3_t diffuse, vec3_t specular,
		float shine, float shineStrength, float transparency);

int renderer_img_getMatGLID(int i);
int renderer_img_getMatWidth(int i);
int renderer_img_getMatHeight(int i);
int renderer_img_getMatBpp(int i);

void renderer_img_loadTGA(char *name, int *glTexID, int *width, int *height, int *bpp);

//http://users.polytech.unice.fr/~buffa/cours/synthese_image/DOCS/Tutoriaux/Nate/ase.html
void renderer_mesh_loadASE(char *name, int collidable);
void renderer_model_drawASE(int index);

void renderer_extensions_init();
void renderer_shaders_init();
GLuint renderer_shader_getProgram();

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
void 	(APIENTRYP glUniform1i) 			(GLint, GLint);
void 	(APIENTRYP glGetProgramiv) 			(GLuint, GLenum, GLint *);
void 	(APIENTRYP glGetProgramInfoLog) 	(GLuint, GLsizei, GLsizei *, GLchar *);
void 	(APIENTRYP glGetShaderiv) 			(GLuint, GLenum, GLint *);
void 	(APIENTRYP glGetShaderInfoLog) 		(GLuint, GLsizei, GLsizei *, GLchar *);

#endif /* RENDERER_H_ */
