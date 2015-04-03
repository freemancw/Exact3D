/*
===========================================================================
File:		renderer_model_ASE.c
Author: 	Clinton Freeman
Created on: Sep 30, 2010
===========================================================================
*/

#include "headers/SDL/SDL_opengl.h"

#include "headers/common.h"
#include "headers/files.h"
#include "headers/mathlib.h"

#include "headers/world_public.h"

#include "headers/renderer_materials.h"
#include "headers/renderer_models.h"

static void loadASE_parseTokens(char **tokens, int numTokens, eboolean collidable, eboolean clamp);
static void loadASE_generateList(int index);

/*
===========================================================================
Data Structures
===========================================================================
*/

//TODO: change this into a generic ase_map_t
/*
typedef struct
{
	char	name[MAX_NAMELENGTH], class[MAX_NAMELENGTH];
	int		subNo;
	float	amount;
	char	bitmap[MAX_FILEPATH];
	char	type[MAX_NAMELENGTH];
	float	uvw_uOffset, uvw_vOffset, uvw_uTiling, uvw_vTiling,
			uvw_angle, uvw_blur, uvw_blurOffset, uvw_noiseAmt,
			uvw_noiseSize, uvw_noiseLevel, uvw_noisePhase;
	char	bitmapFilter[MAX_NAMELENGTH];
}
ase_mapDiffuse_t;
*/

//I'm calling this mapfunction because type, class, and name
//were all taken... =|
typedef enum
{
	MAPFUNC_DIFFUSE,
	MAPFUNC_SPECULAR,
	MAPFUNC_SHINE,
	MAPFUNC_SHINESTRENGTH,
	MAPFUNC_SELFILLUM,
	MAPFUNC_OPACITY,
	MAPFUNC_FILTERCOLOR,
	MAPFUNC_BUMP,
	MAPFUNC_REFLECT,
	MAPFUNC_REFRACT
}
mapFunc;

typedef struct
{
	mapFunc mapFunc;

	char	name[MAX_NAMELENGTH], class[MAX_NAMELENGTH];
	int		subNo;
	float	amount;
	char	bitmap[MAX_FILEPATH];
	char	type[MAX_NAMELENGTH];
	float	uvw_uOffset, uvw_vOffset, uvw_uTiling, uvw_vTiling,
			uvw_angle, uvw_blur, uvw_blurOffset, uvw_noiseAmt,
			uvw_noiseSize, uvw_noiseLevel, uvw_noisePhase;
	char	bitmapFilter[MAX_NAMELENGTH];
}
ase_map_t;

typedef struct
{
	int		id, globalID;
	char	name[MAX_NAMELENGTH], class[MAX_NAMELENGTH];
	vec3_t	ambient, diffuse, specular;
	float	shine, shineStrength, transparency, wireSize;
	char	shading[MAX_NAMELENGTH];
	float	xpFalloff, selfIllum;
	char	falloff[MAX_NAMELENGTH], xpType[MAX_NAMELENGTH];

	int			numMaps;
	ase_map_t 	*maps;
}
ase_material_t;

/* old material
typedef struct
{
	int		id, globalID;
	char	name[MAX_NAMELENGTH], class[MAX_NAMELENGTH];
	vec3_t	ambient, diffuse, specular;
	float	shine, shineStrength, transparency, wireSize;
	char	shading[MAX_NAMELENGTH];
	float	xpFalloff, selfIllum;
	char	falloff[MAX_NAMELENGTH], xpType[MAX_NAMELENGTH];

	//int			numMaps;
	//ase_map_t 	*maps;
	ase_mapDiffuse_t diffuseMap;
}
ase_material_t;
*/

typedef struct
{
	int				materialCount;
	ase_material_t	*list;
}
ase_materialList_t;

typedef struct
{
	int 	faceID;
	int 	A, B, C, AB, BC, CA;
	int 	smoothingGroup, materialID;
	vec3_t	normal;
}
ase_mesh_face_t;

typedef struct
{
	int	tfaceID;
	int a, b, c;
}
ase_mesh_tface_t;

typedef struct
{
	int 	vertexID;
	vec3_t	coords;
	vec3_t	normal;
}
ase_mesh_vertex_t;

typedef struct
{
	int 	vertexID;
	vec3_t	coords;
}
ase_mesh_tvertex_t;

typedef struct
{
	int 	numVertex, numFaces;
	int 	numTVertex, numTVFaces;

	ase_mesh_vertex_t	*vertexList;
	ase_mesh_tvertex_t	*tvertList;
	ase_mesh_face_t		*faceList;
	ase_mesh_tface_t	*tfaceList;
}
ase_mesh_t;

typedef struct
{
	char 		name[MAX_NAMELENGTH];
	ase_mesh_t 	mesh;
	int			materialRef;
}
ase_geomObject_t;

typedef struct
{
	int 				numObjects;
	int					glListID;
	ase_geomObject_t	*objects;
	ase_materialList_t	materialList;
}
ase_model_t;

//Structure Initialization
static void ASE_initMap(ase_map_t *map);
static void ASE_initMaterial(ase_material_t *material);
static void ASE_initMaterialList(ase_materialList_t *materialList);
static void ASE_initFace(ase_mesh_face_t *face);
static void ASE_initTFace(ase_mesh_tface_t *tface);
static void ASE_initVertex(ase_mesh_vertex_t *vertex);
static void ASE_initTVertex(ase_mesh_tvertex_t *tvert);
static void ASE_initMesh(ase_mesh_t *mesh);
static void ASE_initGeomObject(ase_geomObject_t *geomObject);
static void ASE_initModel(ase_model_t *model);

//Debugging
//TODO: change this to generic printMap
//static void ASE_printDiffuse(ase_mapDiffuse_t *diffuse);
static void ASE_printMaterial(ase_material_t *material);
static void ASE_printMaterialList(ase_materialList_t *materialList);
static void ASE_printFace(ase_mesh_face_t *face);
static void ASE_printTFace(ase_mesh_tface_t *tface);
static void ASE_printVertex(ase_mesh_vertex_t *vertex);
static void ASE_printTVertex(ase_mesh_tvertex_t *tvert);
static void ASE_printMesh(ase_mesh_t *mesh);
static void ASE_printGeomObject(ase_geomObject_t *geomObject);
static void ASE_printModel(ase_model_t *model);

//Model storage structures
static ase_model_t 	 modelList[MAX_MODELS];
static modelHandle_t lastPtr = 0;

/*
 * renderer_model_loadASE
 */
modelHandle_t renderer_model_loadASE(char *name, eboolean collidable, eboolean clamp)
{
	FILE 			*file;
	unsigned int	fileSize;
	char 			*fileBuffer, **tokens;
	unsigned int	numTokens, blah;

	//Attempt to load the specified file
	file = fopen(name, "r");

	if(file == NULL)
	{
		printf("Loading ASE: %s, failed. Null file pointer.\n", name);
		return -1;
	}

	//Determine the size of the file
	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	rewind(file);

	//If successful, load the file's contents into a buffer
	fileBuffer = (char *)malloc(sizeof(char) * fileSize);
	blah = fread(fileBuffer, sizeof(char), fileSize+1, file);
	fileBuffer[blah] = '\0';

	//We're done with the file
	fclose(file);

	//Break the file into a new array of tokens
	numTokens = files_tokenizeStr(fileBuffer, " \t\n\r\0", &tokens);
	free(fileBuffer);

	loadASE_parseTokens(tokens, numTokens, collidable, clamp);

	return lastPtr++;
}

/*
 * loadASE_parseTokens
 */
static void loadASE_parseTokens(char **tokens, int numTokens, eboolean collidable, eboolean clamp)
{
	int i, j, curMatID, curObjID, curMapID, curFNormal, curVNormal;
	ase_model_t	*model;
	ase_geomObject_t *currentObject;
	ase_material_t *currentMaterial;
	ase_map_t *currentMap;
	ase_mesh_vertex_t *vertexList;
	ase_mesh_face_t *faceList;
	ase_mesh_tvertex_t *tvertexList;
	ase_mesh_tface_t *tfaceList;
	vec3_t tri[3];

	i = j = curMatID = curObjID = curMapID = curFNormal = curVNormal = 0;

	model = &(modelList[lastPtr]);
	ASE_initModel(model);

	for(i = 0; i < numTokens; i++)
	{
		if(!strcmp(tokens[i], "*MATERIAL_COUNT"))
		{
			model->materialList.materialCount = atoi(tokens[++i]);

			//Allocate enough space for the given number of materials.
			model->materialList.list = (ase_material_t *)malloc(sizeof(ase_material_t) * model->materialList.materialCount);
		}
		else if(!strcmp(tokens[i], "*MATERIAL"))
		{
			curMatID = atoi(tokens[++i]);
			//Grab this material
			currentMaterial = &(model->materialList.list[curMatID]);
			//Initialize it
			ASE_initMaterial(currentMaterial);
			//Set the ID
			currentMaterial->id = curMatID;
		}
		else if(!strcmp(tokens[i], "*MATERIAL_NAME"))
			strcpy(currentMaterial->name, tokens[++i]);
		else if(!strcmp(tokens[i], "*MATERIAL_CLASS"))
			strcpy(currentMaterial->class, tokens[++i]);
		else if(!strcmp(tokens[i], "*MATERIAL_AMBIENT"))
		{
			currentMaterial->ambient[_X] = atoi(tokens[++i]);
			currentMaterial->ambient[_Y] = atoi(tokens[++i]);
			currentMaterial->ambient[_Z] = atoi(tokens[++i]);
		}
		else if(!strcmp(tokens[i], "*MATERIAL_DIFFUSE"))
		{
			currentMaterial->diffuse[_X] = atoi(tokens[++i]);
			currentMaterial->diffuse[_Y] = atoi(tokens[++i]);
			currentMaterial->diffuse[_Z] = atoi(tokens[++i]);
		}
		else if(!strcmp(tokens[i], "*MATERIAL_SPECULAR"))
		{
			currentMaterial->specular[_X] = atoi(tokens[++i]);
			currentMaterial->specular[_Y] = atoi(tokens[++i]);
			currentMaterial->specular[_Z] = atoi(tokens[++i]);
		}
		else if(!strcmp(tokens[i], "*MATERIAL_SHINE"))
			currentMaterial->shine = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*MATERIAL_SHINESTRENGTH"))
			currentMaterial->shineStrength = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*MATERIAL_TRANSPARENCY"))
			currentMaterial->transparency = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*MATERIAL_WIRESIZE"))
			currentMaterial->wireSize = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*MATERIAL_SHADING"))
			strcpy(currentMaterial->shading, tokens[++i]);
		else if(!strcmp(tokens[i], "*MATERIAL_XP_FALLOFF"))
			currentMaterial->xpFalloff = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*MATERIAL_SELFILLUM"))
			currentMaterial->selfIllum = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*MATERIAL_FALLOFF"))
			strcpy(currentMaterial->falloff, tokens[++i]);
		else if(!strcmp(tokens[i], "*MATERIAL_XP_TYPE"))
			strcpy(currentMaterial->xpType, tokens[++i]);
		else if(!strcmp(tokens[i], "*MAP_DIFFUSE"))
		{
			if(currentMaterial->numMaps == 0)
				currentMaterial->maps = (ase_map_t *)malloc(sizeof(ase_map_t));
			else
				currentMaterial->maps = (ase_map_t *)realloc(currentMaterial->maps, sizeof(ase_map_t) * (currentMaterial->numMaps + 1));
			//Grab this map
			currentMap = &(currentMaterial->maps[currentMaterial->numMaps++]);
			//Initialize it
			ASE_initMap(currentMap);
			//Set the "function"
			currentMap->mapFunc = MAPFUNC_DIFFUSE;
		}
		else if(!strcmp(tokens[i], "*MAP_SPECULAR"))
		{
			//Allocate additional space for another map
			currentMaterial->maps = (ase_map_t *)realloc(currentMaterial->maps, sizeof(ase_map_t) * (currentMaterial->numMaps + 1));
			//Grab this map
			currentMap = &(currentMaterial->maps[curMapID++]);
			//Initialize it
			ASE_initMap(currentMap);
			//Set the "function"
			currentMap->mapFunc = MAPFUNC_SPECULAR;
		}
		else if(!strcmp(tokens[i], "*MAP_BUMP"))
		{
			//Allocate additional space for another map
			currentMaterial->maps = (ase_map_t *)realloc(currentMaterial->maps, sizeof(ase_map_t) * (currentMaterial->numMaps + 1));
			//Grab this map
			currentMap = &(currentMaterial->maps[curMapID++]);
			//Initialize it
			ASE_initMap(currentMap);
			//Set the "function"
			currentMap->mapFunc = MAPFUNC_BUMP;
		}
		else if(!strcmp(tokens[i], "*MAP_NAME"))
			strcpy(currentMap->name, tokens[++i]);
		else if(!strcmp(tokens[i], "*MAP_CLASS"))
			strcpy(currentMap->class, tokens[++i]);
		else if(!strcmp(tokens[i], "*MAP_SUBNO"))
			currentMap->subNo = atoi(tokens[++i]);
		else if(!strcmp(tokens[i], "*MAP_AMOUNT"))
			currentMap->amount = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*BITMAP"))
			strcpy(currentMap->bitmap, tokens[++i]);
		else if(!strcmp(tokens[i], "*MAP_TYPE"))
			strcpy(currentMap->type, tokens[++i]);
		else if(!strcmp(tokens[i], "*UVW_U_OFFSET"))
			currentMap->uvw_uOffset = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*UVW_V_OFFSET"))
			currentMap->uvw_vOffset = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*UVW_U_TILING"))
			currentMap->uvw_uTiling = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*UVW_V_TILING"))
			currentMap->uvw_vTiling = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*UVW_ANGLE"))
			currentMap->uvw_angle = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*UVW_BLUR"))
			currentMap->uvw_blur = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*UVW_BLUR_OFFSET"))
			currentMap->uvw_blurOffset = atof(tokens[++i]);
		//TYPO in the exporter!!!!!!
		else if(!strcmp(tokens[i], "*UVW_NOUSE_AMT"))
			currentMap->uvw_noiseAmt = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*UVW_NOISE_SIZE"))
			currentMap->uvw_noiseSize = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*UVW_NOISE_LEVEL"))
			currentMap->uvw_noiseLevel = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*UVW_NOISE_PHASE"))
			currentMap->uvw_noisePhase = atof(tokens[++i]);
		else if(!strcmp(tokens[i], "*BITMAP_FILTER"))
			strcpy(currentMap->bitmapFilter, tokens[++i]);

		else if(!strcmp(tokens[i], "*GEOMOBJECT"))
		{
			//model->numObjects++;
			//curObjID = model->numObjects - 1;

			if(model->numObjects == 0)
				model->objects = (ase_geomObject_t *)malloc(sizeof(ase_geomObject_t));
			else
				model->objects = (ase_geomObject_t *)realloc(model->objects, sizeof(ase_geomObject_t) * (model->numObjects + 1));
			//Grab this object
			currentObject = &(model->objects[model->numObjects++]);
			//Initialize it
			//ASE_initGeomObject(currentObject);
		}
		else if(!strcmp(tokens[i], "*NODE_NAME"))
			strcpy(currentObject->name, tokens[++i]);
		else if(!strcmp(tokens[i], "*MESH_NUMVERTEX"))
		{
			currentObject->mesh.numVertex = atoi(tokens[++i]);
			currentObject->mesh.vertexList =
					(ase_mesh_vertex_t *)malloc(sizeof(ase_mesh_vertex_t) * currentObject->mesh.numVertex);
		}
		else if(!strcmp(tokens[i], "*MESH_NUMFACES"))
		{
			currentObject->mesh.numFaces = atoi(tokens[++i]);
			currentObject->mesh.faceList =
					(ase_mesh_face_t *)malloc(sizeof(ase_mesh_face_t) * currentObject->mesh.numFaces);
		}
		else if(!strcmp(tokens[i], "*MESH_VERTEX_LIST"))
		{
			//Skip *MESH_VERTEX_LIST and {
			i += 2;

			for(j = 0; j < currentObject->mesh.numVertex; j++)
			{
				//Skip *MESH_VERTEX
				i++;

				currentObject->mesh.vertexList[j].vertexID   = atoi(tokens[i++]);
				currentObject->mesh.vertexList[j].coords[_X] = atof(tokens[i++]);
				currentObject->mesh.vertexList[j].coords[_Y] = atof(tokens[i++]);
				currentObject->mesh.vertexList[j].coords[_Z] = atof(tokens[i++]);
			}
		}
		else if(!strcmp(tokens[i], "*MESH_FACE_LIST"))
		{
			//Skip *MESH_FACE_LIST and {
			i += 2;

			for(j = 0; j < currentObject->mesh.numFaces; j++)
			{
				//Skip *MESH_FACE and #:
				i += 2;

				currentObject->mesh.faceList[j].faceID = j;

				//Skip A:
				i++; currentObject->mesh.faceList[j].A = atoi(tokens[i++]);
				//Skip B:
				i++; currentObject->mesh.faceList[j].B = atoi(tokens[i++]);
				//Skip C:
				i++; currentObject->mesh.faceList[j].C = atoi(tokens[i++]);
				//Skip AB:
				i++; currentObject->mesh.faceList[j].AB = atoi(tokens[i++]);
				//Skip BC:
				i++; currentObject->mesh.faceList[j].BC = atoi(tokens[i++]);
				//Skip CA:
				i++; currentObject->mesh.faceList[j].CA = atoi(tokens[i++]);
				//Skip *MESH_SMOOTHING
				//It's possible to not have a smoothing group number, in which case we'll accidentally gobble
				//up too many tokens
				i++;

				//If the next token IS NOT *MESH_MTLID
				if(strcmp(tokens[i], "*MESH_MTLID"))
					//Then grab
					currentObject->mesh.faceList[j].smoothingGroup = atoi(tokens[i++]);

				//Skip *MESH_MTLID
				i++; currentObject->mesh.faceList[j].materialID = atoi(tokens[i++]);
			}
		}
		else if(!strcmp(tokens[i], "*MESH_NUMTVERTEX"))
		{
			currentObject->mesh.numTVertex = atoi(tokens[++i]);
			currentObject->mesh.tvertList =
					(ase_mesh_tvertex_t *)malloc(sizeof(ase_mesh_tvertex_t) * currentObject->mesh.numTVertex);
		}
		else if(!strcmp(tokens[i], "*MESH_TVERTLIST"))
		{
			//Skip *MESH_TVERTLIST and {
			i += 2;

			for(j = 0; j < currentObject->mesh.numTVertex; j++)
			{
				//Skip *MESH_TVERTEX
				i++;

				currentObject->mesh.tvertList[j].vertexID   = atoi(tokens[i++]);
				currentObject->mesh.tvertList[j].coords[_X] = atof(tokens[i++]);
				currentObject->mesh.tvertList[j].coords[_Y] = atof(tokens[i++]);
				currentObject->mesh.tvertList[j].coords[_Z] = atof(tokens[i++]);
			}
		}
		else if(!strcmp(tokens[i], "*MESH_NUMTVFACES"))
		{
			currentObject->mesh.numTVFaces = atoi(tokens[++i]);
			currentObject->mesh.tfaceList =
					(ase_mesh_tface_t *)malloc(sizeof(ase_mesh_tface_t) * currentObject->mesh.numTVFaces);
		}
		else if(!strcmp(tokens[i], "*MESH_TFACELIST"))
		{
			//Skip *MESH_TFACELIST and {
			i += 2;

			for(j = 0; j < currentObject->mesh.numTVFaces; j++)
			{
				//Skip *MESH_TFACE
				i++;

				currentObject->mesh.tfaceList[j].tfaceID = atoi(tokens[i++]);
				currentObject->mesh.tfaceList[j].a = atoi(tokens[i++]);
				currentObject->mesh.tfaceList[j].b = atoi(tokens[i++]);
				currentObject->mesh.tfaceList[j].c = atoi(tokens[i++]);
			}
		}
		else if(!strcmp(tokens[i], "*MESH_FACENORMAL"))
		{
			curFNormal = atoi(tokens[++i]);
			currentObject->mesh.faceList[curFNormal].normal[_X] = atof(tokens[++i]);
			currentObject->mesh.faceList[curFNormal].normal[_Y] = atof(tokens[++i]);
			currentObject->mesh.faceList[curFNormal].normal[_Z] = atof(tokens[++i]);
		}

		else if(!strcmp(tokens[i], "*MESH_VERTEXNORMAL"))
		{
			curVNormal = atoi(tokens[++i]);
			currentObject->mesh.vertexList[curVNormal].normal[_X] = atof(tokens[++i]);
			currentObject->mesh.vertexList[curVNormal].normal[_Y] = atof(tokens[++i]);
			currentObject->mesh.vertexList[curVNormal].normal[_Z] = atof(tokens[++i]);
		}
		else if(!strcmp(tokens[i], "*MATERIAL_REF"))
			currentObject->materialRef = atoi(tokens[++i]);
	}

	//Create OpenGL textures from our materials, and store the global material indices
	for(i = 0; i < model->materialList.materialCount; i++)
	{
		model->materialList.list[i].globalID = renderer_img_createMaterial(model->materialList.list[i].maps[0].bitmap,
				model->materialList.list[i].ambient, model->materialList.list[i].diffuse, model->materialList.list[i].specular,
				model->materialList.list[i].shine, model->materialList.list[i].shineStrength, model->materialList.list[i].transparency, clamp);
	}

	//Correct the mesh's references to point to the global material
	for(i = 0; i < model->numObjects; i++)
		model->objects[i].materialRef = model->materialList.list[model->objects[i].materialRef].globalID;

	//Potentially add triangles to collision list
	if(collidable)
	{
		for(i = 0; i < model->numObjects; i++)
		{
			world_allocCollisionTris(model->objects[i].mesh.numFaces);

			vertexList  = model->objects[i].mesh.vertexList;
			faceList    = model->objects[i].mesh.faceList;

			for(j = 0; j < model->objects[i].mesh.numFaces; j++)
			{
				VectorCopy(vertexList[faceList[j].A].coords, tri[0]);
				VectorCopy(vertexList[faceList[j].B].coords, tri[1]);
				VectorCopy(vertexList[faceList[j].C].coords, tri[2]);

				world_addCollisionTri(tri);
			}
		}
	}

	//Generate a display list for drawing
	model->glListID = glGenLists(1);
	glNewList(model->glListID, GL_COMPILE);
		loadASE_generateList(lastPtr);
	glEndList();
}

/*
 * loadASE_generateList
 */
static void loadASE_generateList(modelHandle_t index)
{
	int i, j;
	vec3_t				tri[3];
	vec3_t 				faceNormal;
	ase_model_t 		*model;
	ase_mesh_vertex_t 	*vertexList;
	ase_mesh_face_t 	*faceList;
	ase_mesh_tface_t 	*tfaceList;
	ase_mesh_tvertex_t 	*tvertList;

	model = &(modelList[index]);

	for(i = 0; i < model->numObjects; i++)
	{
		vertexList  = model->objects[i].mesh.vertexList;
		tvertList   = model->objects[i].mesh.tvertList;
		faceList    = model->objects[i].mesh.faceList;
		tfaceList   = model->objects[i].mesh.tfaceList;

		glBindTexture(GL_TEXTURE_2D,
				renderer_img_getMatGLID(model->objects[i].materialRef));

		for(j = 0; j < model->objects[i].mesh.numFaces; j++)
		{
			glBegin(GL_POLYGON);

				VectorCopy(vertexList[faceList[j].A].coords, tri[0]);
				VectorCopy(vertexList[faceList[j].B].coords, tri[1]);
				VectorCopy(vertexList[faceList[j].C].coords, tri[2]);

				normalFromTri(tri, faceNormal);
				glNormal3fv(faceNormal);

				//glNormal3fv(faceList[j].normal);
				//glNormal3fv(vertexList[faceList[j].A].normal);
				glTexCoord3fv(tvertList[tfaceList[j].a].coords);
				glVertex3fv(vertexList[faceList[j].A].coords);

				//glNormal3fv(vertexList[faceList[j].B].normal);
				glTexCoord3fv(tvertList[tfaceList[j].b].coords);
				glVertex3fv(vertexList[faceList[j].B].coords);

				//glNormal3fv(vertexList[faceList[j].C].normal);
				glTexCoord3fv(tvertList[tfaceList[j].c].coords);
				glVertex3fv(vertexList[faceList[j].C].coords);
			glEnd();
		}
	}
}

/*
 * renderer_model_drawASE
 */
void renderer_model_drawASE(modelHandle_t index)
{
	glCallList(modelList[index].glListID);
}

/*
===========================================================================
Structure Initialization
===========================================================================
*/

static void ASE_initMap(ase_map_t *map)
{
	map->amount = 0.0;
	//memset(bitmap, 0, sizeof(map->bitmap));
	//map->bitmap = NULL;
	//map->bitmapFilter = NULL;
	//map->class = NULL;
	map->mapFunc = 0;
	//map->name = NULL;
	map->subNo = 0;
	//map->type = NULL;
	map->uvw_angle = 0.0;
	map->uvw_uOffset = 0.0;
	map->uvw_vOffset = 0.0;
	map->uvw_uTiling = 0.0;
	map->uvw_vTiling = 0.0;
	map->uvw_blur = 0.0;
	map->uvw_blurOffset = 0.0;
	map->uvw_noiseAmt = 0.0;
	map->uvw_noiseSize = 0.0;
	map->uvw_noiseLevel = 0.0;
	map->uvw_noisePhase = 0.0;
}

static void ASE_initMaterial(ase_material_t *material)
{
	//memset(material, 0, sizeof(ase_material_t));
	material->globalID = 0;
	material->id = 0;
	material->numMaps = 0;
}
static void ASE_initMaterialList(ase_materialList_t *materialList) { memset(materialList, 0, sizeof(ase_materialList_t)); }
static void ASE_initFace(ase_mesh_face_t *face) { memset(face, 0, sizeof(ase_mesh_face_t)); }
static void ASE_initTFace(ase_mesh_tface_t *tface) { memset(tface, 0, sizeof(ase_mesh_tface_t)); }
static void ASE_initVertex(ase_mesh_vertex_t *vertex) { memset(vertex, 0, sizeof(ase_mesh_vertex_t)); }
static void ASE_initTVertex(ase_mesh_tvertex_t *tvert) { memset(tvert, 0, sizeof(ase_mesh_tvertex_t)); }
static void ASE_initMesh(ase_mesh_t *mesh) { memset(mesh, 0, sizeof(ase_mesh_t)); }
static void ASE_initGeomObject(ase_geomObject_t *geomObject) { memset(geomObject, 0, sizeof(ase_geomObject_t)); }
static void ASE_initModel(ase_model_t *model) { memset(model, 0, sizeof(ase_model_t)); }


/*
===========================================================================
Debugging
===========================================================================
*/

/*
 * loadASE_printDiffuse
 */
/*
static void ASE_printDiffuse(ase_mapDiffuse_t *diffuse)
{
	printf("Name: %s\n", 	diffuse->name);
	printf("Class: %s\n", 	diffuse->class);
	printf("SubNo: %d\n", 	diffuse->subNo);
	printf("Amount: %f\n", 	diffuse->amount);
	printf("Bitmap: %s\n", 	diffuse->bitmap);
	printf("Type: %s\n", 	diffuse->type);

	printf("UVW U Offset: %f, UVW V Offset: %f\n", diffuse->uvw_uOffset, diffuse->uvw_vOffset);
	printf("UVW Angle: %f, UVW Blur: %f, UVW Blur Offset: %f\n", diffuse->uvw_angle,
			diffuse->uvw_blur, diffuse->uvw_blurOffset);
	printf("UVW Noise Amount: %f, UVW Noise Level: %f, UVW Noise Phase: %f\n", diffuse->uvw_noiseAmt,
			diffuse->uvw_noiseLevel, diffuse->uvw_noisePhase);
	printf("Bitmap Filter: %s\n", diffuse->bitmapFilter);
}
*/

/*
 * loadASE_printMaterial
 */
static void ASE_printMaterial(ase_material_t *material)
{
	printf("Material ID: %d\n", 		material->id);
	printf("Global ID: %d\n", 			material->globalID);
	printf("Name: %s\n", 				material->name);
	printf("Class: %s\n", 				material->class);

	printf("Ambient: R = %f, G = %f, B = %f\n", material->ambient[_R],
			material->ambient[_G], material->ambient[_B]);
	printf("Diffuse: R = %f, G = %f, B = %f\n", material->diffuse[_R],
			material->diffuse[_G], material->diffuse[_B]);
	printf("Specular: R = %f, G = %f, B = %f\n", material->specular[_R],
			material->specular[_G], material->specular[_B]);

	printf("Shine: %f\n", 				material->shine);
	printf("Shine Strength: %f\n", 		material->shineStrength);
	printf("Transparency: %f\n", 		material->transparency);
	printf("Wire Size: %f\n", 			material->wireSize);
	printf("Shading: %s\n", 			material->shading);
	printf("XP Falloff: %f\n",  		material->xpFalloff);
	printf("Self-Illumination: %f\n", 	material->selfIllum);
	printf("Falloff: %s\n", 			material->falloff);
	printf("XP Type: %s\n", 			material->xpType);

	printf("=======================================================\n");

	//ASE_printDiffuse(&(material->diffuseMap));
}

/*
 * loadASE_printMaterialList
 */
static void ASE_printMaterialList(ase_materialList_t *materialList)
{
	int i;

	printf("Material Count: %d\n", materialList->materialCount);

	for(i = 0; i < materialList->materialCount; i++)
	{
		printf("=======================================================\n");
		ASE_printMaterial(&(materialList->list[i]));
	}
}

/*
 * loadASE_printFace
 */
static void ASE_printFace(ase_mesh_face_t *face)
{
	printf("Face ID: %d\n", 		face->faceID);

	printf("A: %d, B: %d, C: %d, AB: %d, BC: %d, CA: %d\n", face->A, face->B, face->C,
			face->AB, face->BC, face->CA);

	printf("Smoothing Group: %d\n", face->smoothingGroup);
	printf("Material ID: %d\n", 	face->materialID);

	printf("Face Normal: X = %f, Y = %f, Z = %f\n", face->normal[_X], face->normal[_Y], face->normal[_Z]);
}

/*
 * loadASE_printTFace
 */
static void ASE_printTFace(ase_mesh_tface_t *tface)
{
	printf("Texture Face ID: %d\n", tface->tfaceID);
	printf("a: %d, b: %d, c: %d\n", tface->a, tface->b, tface->c);
}

/*
 * loadASE_printVertex
 */
static void ASE_printVertex(ase_mesh_vertex_t *vertex)
{
	printf("Vertex ID: %d\n", vertex->vertexID);

	printf("Coordinates: X = %f, Y = %f, Z = %f\n", vertex->coords[_X],
			vertex->coords[_Y], vertex->coords[_Z]);

	printf("Normal: X = %f, Y = %f, Z = %f\n", vertex->normal[_X],
			vertex->normal[_Y], vertex->normal[_Z]);
}

/*
 * loadASE_printTVertex
 */
static void ASE_printTVertex(ase_mesh_tvertex_t *tvert)
{
	printf("Texture Vertex ID: %d\n", tvert->vertexID);

	printf("Coordinates: X = %f, Y = %f, Z = %f\n", tvert->coords[_X],
			tvert->coords[_Y], tvert->coords[_Z]);
}

/*
 * loadASE_printMesh
 */
static void ASE_printMesh(ase_mesh_t *mesh)
{
	int i;

	printf("Vertex Count: %d\n", mesh->numVertex);
	printf("=======================================================\n");

	for(i = 0; i < mesh->numVertex; i++)
		ASE_printVertex(&(mesh->vertexList[i]));

	printf("Face Count: %d\n", mesh->numFaces);
	printf("=======================================================\n");

	for(i = 0; i < mesh->numFaces; i++)
		ASE_printFace(&(mesh->faceList[i]));

	printf("Texture Vertex Count: %d\n", mesh->numTVertex);
	printf("=======================================================\n");

	for(i = 0; i < mesh->numTVertex; i++)
		ASE_printTVertex(&(mesh->tvertList[i]));


	printf("Texture Face Count: %d\n", mesh->numTVFaces);
	printf("=======================================================\n");

	for(i = 0; i < mesh->numTVFaces; i++)
		ASE_printTFace(&(mesh->tfaceList[i]));
}

/*
 * loadASE_printGeomObject
 */
static void ASE_printGeomObject(ase_geomObject_t *geomObject)
{
	printf("Name: %s\n", geomObject->name);
	printf("Material Reference: %d\n", geomObject->materialRef);
	ASE_printMesh(&(geomObject->mesh));
}

/*
 * loadASE_printModel
 */
static void ASE_printModel(ase_model_t *model)
{
	int i;

	ASE_printMaterialList(&(model->materialList));

	printf("=======================================================\n");

	printf("Object Count: %d\n", model->numObjects);

	for(i = 0; i < model->numObjects; i++)
	{
		printf("=======================================================\n");
		ASE_printGeomObject(&(model->objects[i]));
	}
}

