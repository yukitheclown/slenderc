#include "world.h"
#include "math.h"
#include "octree.h"
#include "bounding_box.h"
#include "window.h"
#include "shaders.h"
#include "object.h"
#include <GL/glew.h>
#include <string.h>
#include <stdlib.h>

static unsigned int vao, posVbo, colorVbo;

static OctreeLeaf *octree = NULL;
static int octreeDivisions, standardOctantWidth;

static Object **offScreenUpdatedObjects = NULL;
static int numOffScreenUpdatedObjects = 0;

static void AddObjectToArr(Object *obj, Object ***objects, int *nObjects){
	*objects = (Object **)realloc(*objects,sizeof(Object *) * ++*nObjects);
	(*objects)[*nObjects-1] = obj;
	obj->AddUser(obj);
}

static void RemoveObjectFromArr(Object *obj, Object ***objects, int *nObjects){
	
	int k;
	for(k = 0; k < *nObjects; k++){
		if((*objects)[k] == obj){

			int f;
			for(f = k; f < *nObjects-1; f++)
				(*objects)[f] = (*objects)[f+1];
			
			*objects = (Object **)realloc(*objects,sizeof(Object *) * --*nObjects);

			obj->RemoveUser(obj);

			break;
		}
	}
}

void World_AddOffScreenUpdatedObject(Object *obj){
	AddObjectToArr(obj, &offScreenUpdatedObjects, &numOffScreenUpdatedObjects);
}

void World_RemoveOffScreenUpdatedObject(Object *obj){
	RemoveObjectFromArr(obj, &offScreenUpdatedObjects, &numOffScreenUpdatedObjects);
}

float World_GetWidth(){
	return octree->width;
}

float World_GetDepth(){
	return octree->depth;
}

float World_GetHeight(){
	return octree->height;
}

void World_InitOctree(Vec3 pos, float size, float octantWidth){

	standardOctantWidth = octantWidth;

	int divisions = roundf(log2((size / octantWidth)));

	octree = (OctreeLeaf *)malloc(sizeof(OctreeLeaf));

	memset(octree, 0, sizeof(OctreeLeaf));

	octree->pos = pos;
	octree->width = size;
	octree->height = size;
	octree->depth = size;
	octree->parent = NULL;
	octreeDivisions = divisions;

	OctreeLeaf_Init(octree, 0, divisions);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLint posLoc = glGetAttribLocation(Shaders_GetProgram(TEXTURELESS_SHADER),SHADERS_POSITION_ATTRIB);
    GLint colorLoc = glGetAttribLocation(Shaders_GetProgram(TEXTURELESS_SHADER), SHADERS_COLOR_ATTRIB);

    if(posLoc != -1){
        glGenBuffers(1, &posVbo);
        glBindBuffer(GL_ARRAY_BUFFER,posVbo);
        glEnableVertexAttribArray(posLoc);
        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    if(colorLoc != -1){
        glGenBuffers(1, &colorVbo);
        glBindBuffer(GL_ARRAY_BUFFER,colorVbo);
        glEnableVertexAttribArray(colorLoc);
        glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glBindVertexArray(0);
}

Vec3 World_GetTopLeft(){
	return octree->pos;
}

void World_RemoveObjectFromOctree(Object *obj){

	if(!obj->inOctant) return;
	
	obj->AddUser(obj);

	OctreeLeaf_Remove(obj->inOctant, obj);
	obj->inOctant = NULL;

	obj->RemoveUser(obj);
}

void World_UpdateObjectInOctree(Object *obj){
	
	if(obj->inOctant){
		
		if(Math_CubeIsCompletelyInside(BoundingBox_GetWorldSpaceCube(&obj->bb), obj->inOctant->cube))
			return;

		obj->AddUser(obj);
		OctreeLeaf_Remove(obj->inOctant, obj);
	 	obj->inOctant = NULL;
		obj->RemoveUser(obj);
	}

	OctreeLeaf_Insert(octree, obj);
}

// int World_GetAllCollisionsRay(Ray ray, float *ret, BoundingBox **closest, Vec2 *normal, IgnoredTypes &ignored){

// 	octree->CheckForCollisionRay(ray, ret, closest, normal, ignored);

// 	return 1;
// }

// int World_GetAllCollisionsRect(cube cube, std::vector<BoundingBox::BoundingBoxObject> *ret){


// 	octree->CheckForCollisionRect(cube, ret);

// 	if(ret->size() > 0) return 1;

// 	return 0;
// }

void World_ResolveCollisions(Object *obj, BoundingBox *box){
	
	OctreeLeaf_ResolveCollisions(octree, obj, box, BoundingBox_GetWorldSpaceCube(box));
}

void World_DrawLines(Vec3 *lines, int num){

	Shaders_UseProgram(TEXTURELESS_SHADER);

	glBindVertexArray(vao);

	Shaders_UpdateViewMatrix();
	Shaders_UpdateModelMatrix();
	Shaders_UpdateProjectionMatrix();

	glLineWidth(1);

	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glBufferData(GL_ARRAY_BUFFER, num*sizeof(Vec3), &lines[0], GL_STATIC_DRAW);

	Vec4 *colors = (Vec4 *)malloc(num * sizeof(Vec4));

	Vec4 white = (Vec4){1,1,1,1};
	
	int k;
	for(k = 0; k < num; k++) colors[k] = white;

	glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
	glBufferData(GL_ARRAY_BUFFER, num*sizeof(Vec4), &colors[0], GL_STATIC_DRAW);

	glDrawArrays(GL_LINE_STRIP, 0, num);
	
	free(colors);

	glBindVertexArray(0);
}

void World_DrawCube(Cube r){

    Vec3 points[8];

    points[0] = (Vec3){r.x, r.y, r.z};
    points[1] = (Vec3){r.x+r.w, r.y, r.z};
    points[2] = (Vec3){r.x+r.w, r.y+r.h, r.z};
    points[3] = (Vec3){r.x, r.y+r.h, r.z};
    points[4] = (Vec3){r.x, r.y, r.z+r.d};
    points[5] = (Vec3){r.x+r.w, r.y, r.z+r.d};
    points[6] = (Vec3){r.x+r.w, r.y+r.h, r.z+r.d};
    points[7] = (Vec3){r.x, r.y+r.h, r.z+r.d};

	Vec3 lines[18];	

    lines[0] = (Vec3){points[0].x, points[0].y, points[0].z};
    lines[1] = (Vec3){points[1].x, points[1].y, points[1].z};
    lines[2] = (Vec3){points[2].x, points[2].y, points[2].z};
    lines[3] = (Vec3){points[3].x, points[3].y, points[3].z};
    lines[4] = (Vec3){points[0].x, points[0].y, points[0].z};
    lines[5] = (Vec3){points[4].x, points[4].y, points[4].z};
    lines[6] = (Vec3){points[5].x, points[5].y, points[5].z};
    lines[7] = (Vec3){points[6].x, points[6].y, points[6].z};
    lines[8] = (Vec3){points[7].x, points[7].y, points[7].z};
    lines[9] = (Vec3){points[4].x, points[4].y, points[4].z};
    lines[10] = (Vec3){points[7].x, points[7].y, points[7].z};
    lines[11] = (Vec3){points[3].x, points[3].y, points[3].z};
    lines[12] = (Vec3){points[7].x, points[7].y, points[7].z};
    lines[13] = (Vec3){points[6].x, points[6].y, points[6].z};
    lines[14] = (Vec3){points[2].x, points[2].y, points[2].z};
    lines[15] = (Vec3){points[6].x, points[6].y, points[6].z};
    lines[16] = (Vec3){points[5].x, points[5].y, points[5].z};
    lines[17] = (Vec3){points[1].x, points[1].y, points[1].z};

    World_DrawLines(lines, 18);
}

static void DrawOctant(OctreeLeaf *quad, Plane *frustumPlanes){

	if(!Math_CheckFrustumCollision(quad->cube, frustumPlanes)) return;

	World_DrawCube(quad->cube);

	int k;
	if(quad->children[0])
		for(k = 0; k < 8; k++) DrawOctant(quad->children[k], frustumPlanes);
}

void World_DrawOctree(){

	Shaders_UseProgram(TEXTURELESS_SHADER);
	Shaders_SetUniformColor((Vec4){0,1,1,1});

	float identity[16];
	Math_Identity(identity);
	Shaders_SetModelMatrix(identity);

	float proj[4][4] = {}, view[16] = {};
	Shaders_GetProjectionMatrix(&proj[0][0]);
	Shaders_GetViewMatrix(&view[0]);
	Math_MatrixMatrixMult(&proj[0][0], &proj[0][0], view);


	Plane frustumPlanes[8];

	frustumPlanes[0] = (Plane){ proj[3][0] + proj[2][0], proj[3][1] + proj[2][1], proj[3][2] + proj[2][2], proj[3][3] + proj[2][3]}; // NEAR
	frustumPlanes[1] = (Plane){ proj[3][0] - proj[2][0], proj[3][1] - proj[2][1], proj[3][2] - proj[2][2], proj[3][3] - proj[2][3]}; // FAR
	frustumPlanes[2] = (Plane){ proj[3][0] + proj[0][0], proj[3][1] + proj[0][1], proj[3][2] + proj[0][2], proj[3][3] + proj[0][3]}; // LEFT
	frustumPlanes[3] = (Plane){ proj[3][0] - proj[0][0], proj[3][1] - proj[0][1], proj[3][2] - proj[0][2], proj[3][3] - proj[0][3]}; // RIGHT
	frustumPlanes[4] = (Plane){ proj[3][0] + proj[1][0], proj[3][1] + proj[1][1], proj[3][2] + proj[1][2], proj[3][3] + proj[1][3]}; // BOTTOM
	frustumPlanes[5] = (Plane){ proj[3][0] - proj[1][0], proj[3][1] - proj[1][1], proj[3][2] - proj[1][2], proj[3][3] - proj[1][3]}; // TOP
	
	int k;
	for(k = 0; k < 6; k++) Math_PlaneNormalize(&frustumPlanes[k]);

	DrawOctant(octree, frustumPlanes);
}

static void DrawBoundingBoxes(BoundingBox *bb){

	Shaders_UseProgram(TEXTURELESS_SHADER);

	int k;
	for(k = 0; k < bb->numChildren; k++)
		DrawBoundingBoxes(&bb->children[k]);

	Shaders_SetUniformColor((Vec4){0,1,0,1});

	World_DrawCube(BoundingBox_GetWorldSpaceCube(bb));
}

void World_Free(){

	if(octree != NULL){
		OctreeLeaf_Free(octree);
		free(octree);
	}

	octree = NULL;
}

void World_Clear(){

	OctreeLeaf_Clear(octree);
}

void World_Render(char drawBoundingBoxes){

	Object **visibleObjects = NULL;
	int numVisibleObjects = 0;

	OctreeLeaf_GetVisibleObjects(octree, &visibleObjects, &numVisibleObjects);

    int k;
	for(k = 0; k < numVisibleObjects; k++){
		if(visibleObjects[k]->Draw){
			Shaders_SetModelMatrix(visibleObjects[k]->matrix);
			visibleObjects[k]->Draw(visibleObjects[k]);
		}
    }

	if(drawBoundingBoxes){

		float identity[16];
		Math_Identity(identity);
		Shaders_SetModelMatrix(identity);

		int k;
		for(k = 0; k < numVisibleObjects; k++){
			Object *object = visibleObjects[k];
			DrawBoundingBoxes(&object->bb);
		}
	}

    if(visibleObjects) free(visibleObjects);
}