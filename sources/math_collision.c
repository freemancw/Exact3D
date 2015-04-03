/*
===========================================================================
File:		math_collision.c
Author: 	Clinton Freeman
Created on: Oct 5, 2010
===========================================================================
*/

#include "headers/common.h"
#include "headers/mathlib.h"

/*
 * normalFromTri
 * Generates a normal vector from the given triangle vertices.
 */
void normalFromTri(vec3_t tri[3], vec3_t normal)
{
	vec3_t vec1, vec2, cross;

	VectorSubtract(tri[0], tri[1], vec1);
	VectorSubtract(tri[1], tri[2], vec2);

	CrossProduct(vec1, vec2, cross);
	VectorNormalize(cross, normal);
}

/*
 * spherePlaneIntersect
 */
eboolean spherePlaneIntersect(vec3_t sCenter, float sRadius, vec3_t pPoint, vec3_t pNormal)
{
	float  heightAbovePlane;
	vec3_t pointToCenter;

	//Create a vector going from a point on the plane to the center
	//of the sphere
	VectorSubtract(sCenter, pPoint, pointToCenter);

	//Calculate the distance of the center to the plane by projecting
	//the newly formed vector onto the plane's normal
	heightAbovePlane = DotProduct(pointToCenter, pNormal);

	//If the height above the plane is not greater than the radius, the sphere
	//and plane are intersecting.
	return (sRadius > fabs(heightAbovePlane));
}

static vec3_t xAxis = {1.0, 0.0, 0.0};
static vec3_t yAxis = {0.0, 1.0, 0.0};
static vec3_t zAxis = {0.0, 0.0, 1.0};

/*
 * triangleAABB
 * Tests an axis-aligned bounding box against a triangle for intersection.
 */
eboolean triangleAABB(vec3_t boxOrigin, vec3_t halfwidths, vec3_t triangle[3], vec3_t tnormal)
{
	int i, j;

	//Triangle moved to box origin
	vec3_t  centeredTri[3];

	//Verticies of the AABB
	vec3_t	boxVertices[8];

	//Edges of the triangle
	vec3_t  triEdges[3];

	//Crossproduct of the edges of the triangle with
	//the cardinal axes
	vec3_t	edgeCross[9];

	//Storage for the projection of each object onto the triangle's normal
	float	triangleProjections[3], boxProjections[8];

	//Maxs and mins for the cardinal axes
	float	xMin, xMax, yMin, yMax, zMin, zMax, nMin, nMax, edgeMax, edgeMin;

	//We already know the max/min for each of the axes tests
	xMin = -halfwidths[_X]; xMax = halfwidths[_X];
	yMin = -halfwidths[_Y]; yMax = halfwidths[_Y];
	zMin = -halfwidths[_Z]; zMax = halfwidths[_Z];

	//=================================================================================================================
	//Specify AABB vertices
	//=================================================================================================================

	//Top of the AABB
	boxVertices[0][_X] = -halfwidths[_X]; boxVertices[0][_Y] = -halfwidths[_Y]; boxVertices[0][_Z] = -halfwidths[_Z];
	boxVertices[1][_X] = -halfwidths[_X]; boxVertices[1][_Y] =  halfwidths[_Y]; boxVertices[1][_Z] = -halfwidths[_Z];
	boxVertices[2][_X] =  halfwidths[_X]; boxVertices[2][_Y] =  halfwidths[_Y]; boxVertices[2][_Z] = -halfwidths[_Z];
	boxVertices[3][_X] =  halfwidths[_X]; boxVertices[3][_Y] = -halfwidths[_Y]; boxVertices[3][_Z] = -halfwidths[_Z];

	//Bottom of the AABB
	boxVertices[4][_X] = -halfwidths[_X]; boxVertices[4][_Y] = -halfwidths[_Y]; boxVertices[4][_Z] = halfwidths[_Z];
	boxVertices[5][_X] = -halfwidths[_X]; boxVertices[5][_Y] =  halfwidths[_Y]; boxVertices[5][_Z] = halfwidths[_Z];
	boxVertices[6][_X] =  halfwidths[_X]; boxVertices[6][_Y] =  halfwidths[_Y]; boxVertices[6][_Z] = halfwidths[_Z];
	boxVertices[7][_X] =  halfwidths[_X]; boxVertices[7][_Y] = -halfwidths[_Y]; boxVertices[7][_Z] = halfwidths[_Z];

	//=================================================================================================================
	//Move triangle to origin
	//=================================================================================================================
	VectorSubtract(triangle[0], boxOrigin, centeredTri[0]);
	VectorSubtract(triangle[1], boxOrigin, centeredTri[1]);
	VectorSubtract(triangle[2], boxOrigin, centeredTri[2]);

	//=================================================================================================================
	//Specify triangle edges
	//=================================================================================================================
	/*VectorSubtract(centeredTri[0], centeredTri[2], triEdges[0]);
	VectorSubtract(centeredTri[1], centeredTri[0], triEdges[1]);
	VectorSubtract(centeredTri[2], centeredTri[1], triEdges[2]);
	*/
	VectorSubtract(centeredTri[0], centeredTri[1], triEdges[0]);
	VectorSubtract(centeredTri[1], centeredTri[2], triEdges[1]);
	VectorSubtract(centeredTri[2], centeredTri[0], triEdges[2]);

	//=================================================================================================================
	//Take cross product between the cardinal axes and triangle edges
	//=================================================================================================================
	CrossProduct(triEdges[0], xAxis, edgeCross[0]);
	CrossProduct(triEdges[1], xAxis, edgeCross[1]);
	CrossProduct(triEdges[2], xAxis, edgeCross[2]);
	CrossProduct(triEdges[0], yAxis, edgeCross[3]);
	CrossProduct(triEdges[1], yAxis, edgeCross[4]);
	CrossProduct(triEdges[2], yAxis, edgeCross[5]);
	CrossProduct(triEdges[0], zAxis, edgeCross[6]);
	CrossProduct(triEdges[1], zAxis, edgeCross[7]);
	CrossProduct(triEdges[2], zAxis, edgeCross[8]);

	//=================================================================================================================
	//Project triangle vertices onto X Axis
	//=================================================================================================================
	triangleProjections[0] = DotProduct(xAxis, centeredTri[0]);
	triangleProjections[1] = DotProduct(xAxis, centeredTri[1]);
	triangleProjections[2] = DotProduct(xAxis, centeredTri[2]);

	//Determine if the shapes overlap on the X Axis, if not we've found a separating axis.
	if( ( (triangleProjections[0] > xMax) && (triangleProjections[1] > xMax) && (triangleProjections[2] > xMax) ) ||
		( (triangleProjections[0] < xMin) && (triangleProjections[1] < xMin) && (triangleProjections[2] < xMin) ))
		return efalse;

	//=================================================================================================================
	//Project triangle verticies onto Y Axis
	//=================================================================================================================
	triangleProjections[0] = DotProduct(yAxis, centeredTri[0]);
	triangleProjections[1] = DotProduct(yAxis, centeredTri[1]);
	triangleProjections[2] = DotProduct(yAxis, centeredTri[2]);

	//Determine if the shapes overlap on the Y Axis, if not we've found a separating axis.
	if( ( (triangleProjections[0] > yMax) && (triangleProjections[1] > yMax) && (triangleProjections[2] > yMax) ) ||
		( (triangleProjections[0] < yMin) && (triangleProjections[1] < yMin) && (triangleProjections[2] < yMin) ))
		return efalse;

	//=================================================================================================================
	//Project triangle verticies onto Z Axis
	//=================================================================================================================
	triangleProjections[0] = DotProduct(zAxis, centeredTri[0]);
	triangleProjections[1] = DotProduct(zAxis, centeredTri[1]);
	triangleProjections[2] = DotProduct(zAxis, centeredTri[2]);

	//Determine if the shapes overlap on the Z Axis, if not we've found a separating axis.
	if( ( (triangleProjections[0] > zMax) && (triangleProjections[1] > zMax) && (triangleProjections[2] > zMax) ) ||
		( (triangleProjections[0] < zMin) && (triangleProjections[1] < zMin) && (triangleProjections[2] < zMin) ))
		return efalse;

	//=================================================================================================================
	//Project triangle verticies onto it's normal vector
	//=================================================================================================================
	triangleProjections[0] = DotProduct(tnormal, centeredTri[0]);
	triangleProjections[1] = DotProduct(tnormal, centeredTri[1]);
	triangleProjections[2] = DotProduct(tnormal, centeredTri[2]);

	//=================================================================================================================
	//Project the AABB onto the triangle's normal vector
	//=================================================================================================================
	boxProjections[0] = DotProduct(tnormal, boxVertices[0]); boxProjections[1] = DotProduct(tnormal, boxVertices[1]);
	boxProjections[2] = DotProduct(tnormal, boxVertices[2]); boxProjections[3] = DotProduct(tnormal, boxVertices[3]);
	boxProjections[4] = DotProduct(tnormal, boxVertices[4]); boxProjections[5] = DotProduct(tnormal, boxVertices[5]);
	boxProjections[6] = DotProduct(tnormal, boxVertices[6]); boxProjections[7] = DotProduct(tnormal, boxVertices[7]);

	//=================================================================================================================
	//Calculate the max/min box projection magnitudes
	//=================================================================================================================
	nMin = nMax = boxProjections[0];
	for(i = 0; i < 8; i++)
	{
		if(boxProjections[i] > nMax)
			nMax = boxProjections[i];

		if(boxProjections[i] < nMin)
			nMin = boxProjections[i];
	}

	//Determine if the shapes overlap, if not we've found a separating axis.
	if( ( (triangleProjections[0] > nMax) && (triangleProjections[1] > nMax) && (triangleProjections[2] > nMax) ) ||
		( (triangleProjections[0] < nMin) && (triangleProjections[1] < nMin) && (triangleProjections[2] < nMin) ))
		return efalse;

	//=================================================================================================================
	//For each of the cross products, project both the AABB and the triangle onto the cross product vector.
	//=================================================================================================================
	for(i = 0; i < 9; i++)
	{
		VectorNormalize(edgeCross[i], edgeCross[i]);

		//Project the triangle
		triangleProjections[0] = DotProduct(edgeCross[i], centeredTri[0]);
		triangleProjections[1] = DotProduct(edgeCross[i], centeredTri[1]);
		triangleProjections[2] = DotProduct(edgeCross[i], centeredTri[2]);

		//Project the AABB
		boxProjections[0] = DotProduct(edgeCross[i], boxVertices[0]); boxProjections[1] = DotProduct(edgeCross[i], boxVertices[1]);
		boxProjections[2] = DotProduct(edgeCross[i], boxVertices[2]); boxProjections[3] = DotProduct(edgeCross[i], boxVertices[3]);
		boxProjections[4] = DotProduct(edgeCross[i], boxVertices[4]); boxProjections[5] = DotProduct(edgeCross[i], boxVertices[5]);
		boxProjections[6] = DotProduct(edgeCross[i], boxVertices[6]); boxProjections[7] = DotProduct(edgeCross[i], boxVertices[7]);

		//Calculate the max/min box projection magnitudes
		nMin = nMax = boxProjections[0];
		for(j = 0; j < 8; j++)
		{
			if(boxProjections[j] > nMax)
				nMax = boxProjections[j];

			if(boxProjections[j] < nMin)
				nMin = boxProjections[j];
		}

		//Determine if the shapes overlap, if not we've found a separating axis.
		if( ( (triangleProjections[0] > nMax) && (triangleProjections[1] > nMax) && (triangleProjections[2] > nMax) ) ||
			( (triangleProjections[0] < nMin) && (triangleProjections[1] < nMin) && (triangleProjections[2] < nMin) )) {
			return efalse;
		}
	}

	return etrue;
}
