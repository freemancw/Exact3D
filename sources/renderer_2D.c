/*
===========================================================================
File:		renderer_2d.c
Author: 	Clinton Freeman
Created on: Oct 14, 2010
===========================================================================
*/

#include "headers/SDL/SDL_opengl.h"
#include "headers/SDL/SDL_ttf.h"

#include "headers/common.h"
#include "headers/mathlib.h"

#include "headers/renderer_2D.h"
#include "headers/renderer_materials.h"

typedef struct
{
	char 		*str, *fontFace;
	int	 		fontPoint, glID;
	int			width, height;
	GLfloat		texCoords[4];
	SDL_Color 	color;
	SDL_Surface *surface;
	TTF_Font	*font;
}
stringTexture_t;

static int testglID;

static stringTexture_t testText;

static int 	minPowerOfTwo(int);
static void enter2D();
static void exit2D();
static void glTextureFromSurface(stringTexture_t *);

/*
 * renderer_2D_init
 */
void renderer_2D_init()
{
	vec3_t empty = {0.0, 0.0, 0.0};
	TTF_Init();
    atexit(TTF_Quit);

    testText.str 			= "Torso PCA Frame";
    testText.fontFace		= "gui/fonts/Enigma__2.ttf";
    testText.fontPoint 		= 24;
    testText.color.r 		= 255;
    testText.color.g 		= 255;
    testText.color.b 		= 255;
    testText.color.unused 	= 0;
    testText.font			= TTF_OpenFont(testText.fontFace, testText.fontPoint);
    testText.surface		= TTF_RenderText_Blended(testText.font, testText.str, testText.color);

    glTextureFromSurface(&testText);

    testglID = renderer_img_createMaterial("gui/test.tga",
    		empty, empty, empty, 0.0, 0.0, 0.0, efalse);

    testglID = renderer_img_getMatGLID(testglID);
}

/*
 * renderer_draw2D
 */
void renderer_draw2D(void)
{
	static int 	x = 16;
	static int 	y = 16;

	enter2D();

	glBindTexture(GL_TEXTURE_2D, testglID);


	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0.0, 0.0);
			glVertex2i(0, 0);
		glTexCoord2f(1.0, 0.0);
			glVertex2i(WINDOW_WIDTH, 0);
		glTexCoord2f(0.0, 1.0);
			glVertex2i(0, WINDOW_HEIGHT);
		glTexCoord2f(1.0, 1.0);
			glVertex2i(WINDOW_WIDTH, WINDOW_HEIGHT);
	glEnd();



	glBindTexture(GL_TEXTURE_2D, testText.glID);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(testText.texCoords[0], testText.texCoords[1]);
			glVertex2i(x, y);
		glTexCoord2f(testText.texCoords[2], testText.texCoords[1]);
			glVertex2i(x + testText.surface->w, y);
		glTexCoord2f(testText.texCoords[0], testText.texCoords[3]);
			glVertex2i(x, y + testText.surface->h);
		glTexCoord2f(testText.texCoords[2], testText.texCoords[3]);
			glVertex2i(x + testText.surface->w, y + testText.surface->h);
	glEnd();

	exit2D();
}

/*
 * glTextureFromSurface
 */
static void glTextureFromSurface(stringTexture_t *strTex)
{
	int 			w, h;
	SDL_Rect		area;
	SDL_Surface		*image;
	Uint8			saved_alpha;
	Uint32			saved_flags;

	//Use the surface width and height expanded to powers of 2
	w = minPowerOfTwo(strTex->surface->w);
	h = minPowerOfTwo(strTex->surface->h);

	strTex->texCoords[0] = 0.0f;	//Min X
	strTex->texCoords[1] = 0.0f;	//Min Y
	strTex->texCoords[2] = (GLfloat)strTex->surface->w / w;	//Max X
	strTex->texCoords[3] = (GLfloat)strTex->surface->h / h;	//Max Y

	//Create a blank RGBA surface
	image = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
			0x000000FF,
			0x0000FF00,
			0x00FF0000,
			0xFF000000
#else
			0xFF000000,
			0x00FF0000,
			0x0000FF00,
			0x000000FF
#endif
	);

	if(image == NULL)
		return;

	//Save the alpha blending attributes
	saved_flags = (strTex->surface->flags) & (SDL_SRCALPHA|SDL_RLEACCELOK);
	saved_alpha = strTex->surface->format->alpha;

	if((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA)
		SDL_SetAlpha(strTex->surface, 0, 0);

	//Copy the string image into the properly sized surface
	area.x = 0; area.y = 0;
	area.w = strTex->surface->w;
	area.h = strTex->surface->h;
	SDL_BlitSurface(strTex->surface, &area, image, &area);

	//Restore the alpha blending attributes
	if ((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA)
		SDL_SetAlpha(strTex->surface, saved_flags, saved_alpha);

	//Create an OpenGL texture for the image
	glGenTextures(1, &(strTex->glID));
	glBindTexture(GL_TEXTURE_2D, strTex->glID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h,
			0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);

	//No longer needed...
	SDL_FreeSurface(image);
}

/*
 * minPowerOfTwo
 */
static int minPowerOfTwo(int n)
{
	int value = 1;

	while(value < n) value <<= 1;

	return value;
}

/*
 * enter2D
 */
static void enter2D()
{
	SDL_Surface *screen;

	//Perserve and adjust gl state
	glPushAttrib(GL_ENABLE_BIT);

	glDisable(GL_DEPTH_TEST);  glDisable(GL_CULL_FACE);
	glEnable (GL_TEXTURE_2D);  glEnable (GL_BLEND);

	//Alpha blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	screen = SDL_GetVideoSurface();
	glViewport(0, 0, screen->w, screen->h);

	glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();

	glOrtho(0.0, (GLdouble)screen->w, (GLdouble)screen->h, 0.0, 0.0, 1.0);

	glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

	glColor3f(1.0, 1.0, 1.0);
}

/*
 * exit2D
 */
static void exit2D()
{
	glMatrixMode(GL_MODELVIEW);  glPopMatrix();
	glMatrixMode(GL_PROJECTION); glPopMatrix();

	//Restore gl state
	glPopAttrib();

	glMatrixMode(GL_MODELVIEW);
}
