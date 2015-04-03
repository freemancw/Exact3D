/*
===========================================================================
File:		renderer_main.c
Author: 	Clinton Freeman
Created on: Sep 29, 2010
===========================================================================
*/

#include "headers/SDL/SDL.h"
#include "headers/SDL/SDL_opengl.h"

#include "headers/common.h"
#include "headers/mathlib.h"

//TODO: get rid of this
#include "headers/player.h"

#include "headers/renderer_public.h"
#include "headers/renderer_extensions.h"
#include "headers/renderer_shaders.h"
#include "headers/renderer_materials.h"
#include "headers/renderer_models.h"
#include "headers/renderer_camera.h"
#include "headers/renderer_2D.h"

//#define CONVEXHULL

static modelHandle_t skyModel, worldModel, shotgunModel;

#ifdef CONVEXHULL

//INPUT - array of randomized points in the plane, statically allocated for now
#define NUMPOINTS 16
vec3_t pointArray[NUMPOINTS];

//EDGE DEFINITIONS
typedef struct
{
	int 		degree;
	eboolean 	onHull;
}
edgeInfo_t;

typedef struct
{
	//This is "global" data that will be the same for edges such as:
	//{0, 1} and {1, 0}
	//{2, 3} and {3, 2}
	//etc.
	edgeInfo_t 	*global;

	//This is "local" data that will be tailored to a particular triangle
	int			from, to;
	vec3_t 		vector, normal;
	vec_t  		baseline;
}
edge_t;

//TRIANGLE DEFINITIONS
typedef struct
{
	//Each triangle has 3 unique edges for storing the vector, normal, etc.
	//These edges also contain a pointer to "global" properties
	edge_t edges[3];
	vec3_t normal;
}
triangle_t;

//DATA STORAGE
edgeInfo_t globalEdges[NUMPOINTS][NUMPOINTS];
triangle_t triList[256]; //this number is random, just sufficiently big ATM
int triCount;

/*
 * generateRandomPoints
 * Randomly generate NUMPOINTS # of position vectors in the XY plane.
 */
void generateRandomPoints()
{
	int i;

	//Initialize random number generator using current time
	srand(SDL_GetTicks());

	for(i = 0; i < NUMPOINTS; i++)
	{
		pointArray[i][_X] = (float)(rand()%1000)/10.0 - 50.0;
		pointArray[i][_Y] = (float)(rand()%1000)/10.0 - 50.0;
		pointArray[i][_Z] = 0.0;
	}
}

/*
 * getGlobalEdge
 * Returns a reference to a global edge structure for a given
 * pair of vertices.
 */
edgeInfo_t *getGlobalEdge(int from, int to)
{
	int i, j;

	if(to > from)
	{
		i = to;
		j = from;
	}
	else
	{
		i = from;
		j = to;
	}

	return &(globalEdges[i][j]);
}

/*
 * createEdge
 * Correctly creates the following values for an edge:
 * to, from, global, global->degree, global->onHull, vector
 */
void createEdge(int from, int to, edge_t *edge)
{
	//Global information
	edge->from = from; edge->to = to;
	//edge->global = &(globalEdges[from][to]);
	edge->global = getGlobalEdge(from, to);
	//Increase degree and determine if edge is part of multiple triangles
	edge->global->degree++;
	if(edge->global->degree > 1)
		edge->global->onHull = efalse;
	else
		edge->global->onHull = etrue;
	//Create the actual edge vector
	VectorSubtract(pointArray[to], pointArray[from], edge->vector);
}

/*
 * createEdgeNormal
 * Creates a normalized normal for a particular edge. Also responsible for
 * computing a "baseline" projection value using the "from" edge vertex.
 */
void createEdgeNormal(edge_t *edge, vec3_t triNormal)
{
	//Compute and normalize edge normal
	CrossProduct(edge->vector, triNormal, edge->normal);
	VectorNormalize(edge->normal, edge->normal);
	//Compute "baseline" value
	edge->baseline = DotProduct(edge->normal, pointArray[edge->from]);
}

/*
 * createTriangle
 * Correctly fills in a triangle structure given 3 ordered points
 * and an index into the global triangle array.
 */
void createTriangle(int v0, int v1, int v2, int triIndex)
{
	triangle_t *t = &(triList[triIndex]);

	//Create basic edge information
	createEdge(v0, v1, &(t->edges[0]));
	createEdge(v1, v2, &(t->edges[1]));
	createEdge(v2, v0, &(t->edges[2]));

	//Compute and normalize triangle normal
	CrossProduct(t->edges[0].vector, t->edges[1].vector, t->normal);
	VectorNormalize(t->normal, t->normal);

	//Use triangle normal to generate edge normals and baselines
	createEdgeNormal(&(t->edges[0]), t->normal);
	createEdgeNormal(&(t->edges[1]), t->normal);
	createEdgeNormal(&(t->edges[2]), t->normal);
}

/*
 * initializeHull
 * Create a triangle from the first three points in pointArray.
 */
void initializeHull()
{
	int i, j;

	triCount = 0;

	for(i = 0; i < NUMPOINTS; i++)
	{
		for(j = 0; j < NUMPOINTS; j++)
		{
			globalEdges[i][j].degree = 0;
			globalEdges[i][j].onHull = efalse;
		}
	}

	createTriangle(0, 1, 2, 0);
	triCount++;
}

/*
 * generateHull
 */
void generateHull()
{
	int i, j, k, newTris;
	vec3_t *new;
	triangle_t *t;
	edge_t *e;

	//For each point, starting at the 4th
	for(i = 3; i < NUMPOINTS; i++)
	{
		//Grab it
		new = &(pointArray[i]);

		newTris = 0;

		//For each triangle
		for(j = 0; j < triCount; j++)
		{
			//Grab it
			t = &(triList[j]);

			//For each edge
			for(k = 0; k < 3; k++)
			{
				//Grab it
				e = &(t->edges[k]);

				//If it's not on the hull, we don't care
				if(!e->global->onHull)
					continue;

				//Is the new vertex "outside" this edge?
				if(DotProduct(e->normal, (*new)) > e->baseline)
				{
					//Create a new triangle
					createTriangle(e->from, e->to, i, triCount + newTris);
					newTris++;
				}
			}
		}

		triCount += newTris;
	}
}

#endif

/*
===========================================================================
INITIALIZATION
===========================================================================
*/

/*
 * renderer_gl_init()
 */
static void renderer_gl_init()
{
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearDepth(1.0);

	glPointSize(8.0);
	glLineWidth(2.0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	//glEnable (GL_LINE_SMOOTH);
	//glEnable(GL_POINT_SMOOTH);

#ifndef CONVEXHULL

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


#endif
}

/*
 * renderer_loadGameMeshes
 */
static void renderer_loadGameMeshes()
{
	skyModel     = renderer_model_loadASE("models/skybox.ASE",  efalse, etrue);
	worldModel   = renderer_model_loadASE("models/egypt.ASE",	 etrue, efalse);
	shotgunModel = renderer_model_loadASE("models/shotgun.ASE", efalse, efalse);
}

/*
 * renderer_init
 */
void renderer_init()
{
	renderer_gl_init();
	renderer_extensions_init();
	renderer_shaders_init();
	camera_init();
	renderer_2D_init();

#ifndef CONVEXHULL
	renderer_loadGameMeshes();
#endif

#ifdef CONVEXHULL

	//CONVEX HULL STUFF
	//Begin by generating a random set of points
	generateRandomPoints();

	//Then create a triangle that connects the first three vertices
	initializeHull();
	generateHull();

#endif
}

/*
===========================================================================
DRAWING
===========================================================================
*/

/*
 * renderer_drawSky
 * Note: modelview is assumed to have been properly calculated.
 * TODO: fix lines/clamping??
 */
static void renderer_drawSky()
{
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//Turn off world shader
	glUseProgram(0);
	renderer_model_drawASE(skyModel);
}

/*
 * renderer_drawWorld
 * Note: modelview is assumed to have been properly calculated.
 */
static void renderer_drawWorld()
{
	int prog, lightPos, cameraPos, viewDir,
		cameraRange, screenSize, playerPos;
	vec2_t cRange, sSize;
	vec3_t cPos, vDir, lPos;

	//Grab the current shader program
	prog = renderer_shader_getProgram();

	//Get the view direction and flip it
	player_getForwardVec(vDir);
	VectorNormalize(vDir, vDir);
	//VectorScale(vDir, 32.0, vDir);

	//VectorInverse(vDir);

	//Get the camera's position
	player_getLerpPosition(cPos);
	VectorAdd(cPos, vDir, lPos);


	//Turn on global world shader
	glUseProgram(prog);
	playerPos = glGetUniformLocation(prog, "playerPos");
	glUniform3f(playerPos, cPos[_X], cPos[_Y], cPos[_Z]);
	//lightPos  = glGetUniformLocation(prog, "lightPos");
	//cameraPos = glGetUniformLocation(prog, "cameraPos");
	//viewDir   = glGetUniformLocation(prog, "viewDir");
	//cameraRange = glGetUniformLocation(prog, "camerarange");
	//screenSize = glGetUniformLocation(prog, "screensize");
	//cRange[_X] = 0.5; cRange[_Y] = 2048.0;
	//glUniform2f(cameraRange, cRange[_X], cRange[_Y]);
	//glUniform2f(screenSize, WINDOW_WIDTH, WINDOW_HEIGHT);
	//glUniform3f(lightPos, cPos[_X], cPos[_Y], cPos[_Z]);
	//glUniform3f(cameraPos, cPos[_X], cPos[_Y], cPos[_Z]);
	//glUniform3f(viewDir, vDir[_X], vDir[_Y], vDir[_Z]);

	renderer_model_drawASE(worldModel);
}

/*
 * renderer_drawFPGeom
 * Draws "first person geometry," i.e. a gun, torch, etc.
 * Note: responsible for calculating modelview
 */
static void renderer_drawFPGeom()
{
	static float bob = 0.0;
	float pSpeed;

	player_getForwardSpeed(&pSpeed);
	bob += pSpeed/175.0;

	if(bob > 100.0)
		bob = 0.0;
	//Turn off world shader
	glUseProgram(0);

	//Set up modelview
	glPushMatrix();
		glLoadIdentity();
		glTranslatef(8.0+ (sin_t(bob) * .25), -14.0 + (sin_t(bob) * .5), -6.0);
		//glTranslatef(0.0, -2.5, -18.0);
		glScalef(1.2, 1.0, .3);
		//glScalef(4.5, 3.5, 1.0);
		glRotatef(90.0, 0.0, 0.0, 1.0);
		glRotatef(90.0, 0.0, 1.0, 0.0);
		//Draw it
		renderer_model_drawASE(shotgunModel);
	glPopMatrix();
}

/*
 * renderer_drawFrame
 */
void renderer_drawFrame()
{
#ifdef CONVEXHULL

	//CONVEX HULL STUFF
	int 		i, j;
	triangle_t 	*t;
	edge_t 		*e;
	vec3_t		*p0, *p1, *p2;
	vec3_t		triCenter, triNorm, triNormEnd;
	vec3_t		midPoint, edgeVector, edgeNormEnd;

#endif

	int i, j;

	static float skycolor = 0.0f;

	glClearColor(skycolor, skycolor, 0, 1);
	skycolor += 0.01;
	if(skycolor > 1.0) skycolor = 0.0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera_rotateModelview();

	//renderer_drawSky();

	glClear(GL_DEPTH_BUFFER_BIT);

	camera_translateModelview();

#ifndef CONVEXHULL

	renderer_drawWorld();

	glClear(GL_DEPTH_BUFFER_BIT);

	//Note: responsible for setting up modelview
	renderer_drawFPGeom();

#endif

	renderer_draw2D();

#ifdef CONVEXHULL

	//CONVEX HULL STUFF
	glUseProgram(0);
	//DRAW THE POINTS
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_POINTS);

	for(i = 0; i < NUMPOINTS; i++)
	{
		//Highlight the first three vertices
		if(i < 3)
			glColor3f((1.0/255.0)*0.0, (1.0/255.0)*107.0, (1.0/255.0)*252.0);
		else
			glColor3f(1.0, 1.0, 1.0);

		glVertex3fv(pointArray[i]);
	}

	glEnd();

	//DRAW THE TRIANGLES
	//For each triangle
	for(i = 0; i < triCount; i++)
	{
		//Grab it
		t = &(triList[i]);

		//Grab its points
		p0 = &(pointArray[t->edges[0].from]);
		p1 = &(pointArray[t->edges[0].to]);
		p2 = &(pointArray[t->edges[1].to]);

		//Compute the centroid
		VectorAdd((*p0), (*p1), triCenter);
		VectorAdd(triCenter, (*p2), triCenter);
		VectorScale(triCenter, (1.0/3.0), triCenter);

		//Add the scaled normal to the centroid to find the endpoint
		VectorScale(t->normal, 4.0, triNorm);
		VectorAdd(triCenter, triNorm, triNormEnd);

		//Draw the triangle normal
		glBegin(GL_LINES);
			glVertex3fv(triCenter);
			glVertex3fv(triNormEnd);
		glEnd();

		//For each edge
		for(j = 0; j < 3; j++)
		{
			//Grab it
			e = &(t->edges[j]);

			//Compute the midpoint
			VectorScale(e->vector, 0.5, edgeVector);
			VectorAdd(pointArray[e->from], edgeVector, midPoint);
			VectorScale(e->normal, 4.0, edgeVector);
			VectorAdd(midPoint, edgeVector, edgeNormEnd);

			//Draw the edge normal
			glBegin(GL_LINES);
				glVertex3fv(midPoint);
				glVertex3fv(edgeNormEnd);
			glEnd();

			//Draw the edge line (in red if on hull)
			if(e->global->onHull)
			{
				glColor3f((1.0/255.0)*191.0, (1.0/255.0)*218.0, (1.0/255.0)*255.0);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(2, 0xAAAA);
			}

			glBegin(GL_LINES);
				glVertex3fv(pointArray[e->from]);
				glVertex3fv(pointArray[e->to]);
			glEnd();

			//Reset to white
			glColor3f(1.0, 1.0, 1.0);
			glDisable(GL_LINE_STIPPLE);
		}
	}

#endif

	SDL_GL_SwapBuffers();
}
