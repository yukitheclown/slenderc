#include "octree.h"
#include "shaders.h"
#include "window.h"
#include "object.h"

void OctreeLeaf_Init(OctreeLeaf *o, int index, int divisions){

 	o->numObjects = 0;
	o->level = 0;

	if(o->parent != NULL){
		o->width = o->parent->width / 2;
		o->height = o->parent->height / 2;
		o->depth = o->parent->depth / 2;
		o->pos.x = o->parent->pos.x + ((index % 2) * o->width);
		o->pos.y = o->parent->pos.y + (round((index % 4) / 2) * o->height);
		o->pos.z = o->parent->pos.z + (round(index / 4) * o->depth);
		o->level = o->parent->level+1;
	}
	
	o->cube = (Cube){o->pos.x, o->pos.y, o->pos.z, o->width, o->height, o->depth};

	int k;
	if(o->level <= divisions){
		for(k = 0; k < 8; k++){
			o->children[k] = (OctreeLeaf *)malloc(sizeof(OctreeLeaf));
			memset(o->children[k], 0, sizeof(OctreeLeaf));
			o->children[k]->parent = o;
			OctreeLeaf_Init(o->children[k], k, divisions);
		}
	}
}

int OctreeLeaf_Insert(OctreeLeaf *o, Object *obj){

	if(!Math_CubeIsCompletelyInside(BoundingBox_GetWorldSpaceCube(&obj->bb), o->cube) && o->level != 0)
		return 0;

	if(o->children[0]){

		int k;
		for(k = 0; k < 8; k++)
			if(OctreeLeaf_Insert(o->children[k], obj))
				return 1;

	}

	o->objects = (Object **)realloc(o->objects, sizeof(Object *) * ++o->numObjects);

	o->objects[o->numObjects-1] = obj;

	obj->inOctant = o;

	obj->AddUser(obj);

	return 1;
}

void OctreeLeaf_Remove(OctreeLeaf *o, Object *obj){

    int k;
    for(k = 0; k < o->numObjects; k++){
        if(o->objects[k] == obj){

            int f;
            for(f = k; f < o->numObjects-1; f++)
                o->objects[f] = o->objects[f+1];

            --o->numObjects;
            
            o->objects = (Object **)realloc(o->objects, sizeof(Object *) * o->numObjects);

            break;
        }
    }

    obj->RemoveUser(obj);

	obj->inOctant = NULL;
}

void OctreeLeaf_MoveInto(OctreeLeaf *into, OctreeLeaf *oct){

	int k;
	for(k = 0; k < (int)oct->numObjects; k++){

		Object *obj = oct->objects[k];

	 	obj->inOctant = NULL;
		OctreeLeaf_Insert(into, obj);

	    obj->RemoveUser(obj);
	}

	free(oct->objects);
	oct->objects = NULL;
	oct->numObjects = 0;

	if(oct->children[0]){
		for(k = 0; k < 8; k++)
			OctreeLeaf_MoveInto(into,oct->children[k]);
	}
}

void OctreeLeaf_ResolveCollisions(OctreeLeaf *o, Object *obj, BoundingBox *box, Cube minCube){

	if(!Math_CheckCollisionCube(o->cube, minCube))
		return;

	int k;
	for(k = 0; k < o->numObjects; k++){

		BoundingBox *bb = &o->objects[k]->bb;

		if(!Math_CheckCollisionCube(BoundingBox_GetWorldSpaceCube(bb), minCube) || bb == box || obj == o->objects[k]) continue;

		BoundingBox_ResolveCollision(obj, box, o->objects[k], bb);
	}

	if(o->children[0])
		for(k = 0; k < 8; k++)
			OctreeLeaf_ResolveCollisions(o->children[k], obj, box, minCube);
}

void OctreeLeaf_Clear(OctreeLeaf *o){

	int k;
	for(k = 0; k < o->numObjects; k++){
		o->objects[k]->inOctant = NULL;
		o->objects[k]->RemoveUser(o->objects[k]);
	}

	if(o->objects) free(o->objects);

	o->objects = NULL;
	o->numObjects = 0;

	if(o->children[0]){
		for(k = 0; k < 8; k++){
			OctreeLeaf_Clear(o->children[k]);
		}
	}
}

void OctreeLeaf_Free(OctreeLeaf *o){

	int k;
	for(k = 0; k < o->numObjects; k++){

		o->objects[k]->inOctant = NULL;
		o->objects[k]->RemoveUser(o->objects[k]);
	}

	if(o->objects) free(o->objects);

	o->objects = NULL;
	o->numObjects = 0;

	if(o->children[0]){
		for(k = 0; k < 8; k++){
			OctreeLeaf_Free(o->children[k]);
			free(o->children[k]);
		}
	}
}

void OctreeLeaf_ResolveCollisionsRay(OctreeLeaf *o, Ray ray, float *dist,  BoundingBox **closest){

	if(Math_CubeCheckCollisionRay(o->cube, ray) == HUGE_VAL)
		return;

	int k;
	for(k = 0; k < o->numObjects; k++){

		BoundingBox *bb;

		float distance = BoundingBox_CheckCollisionRay(&o->objects[k]->bb, ray, &bb);

		if(distance < *dist){
			*dist = distance;
			*closest = bb;
		}
	}

	if(o->children[0])
		for(k = 0; k < 8; k++)
			OctreeLeaf_ResolveCollisionsRay(o->children[k], ray, dist, closest);
}

static void FindViewCollisions(OctreeLeaf *o, Object ***ret, int *nObjects, Plane *frustumPlanes){

	if(!o || !Math_CheckFrustumCollision(o->cube, frustumPlanes)) return;

	int k;
	for(k = 0; k < (int)o->numObjects; k++){

		if(Math_CheckFrustumCollision(BoundingBox_GetWorldSpaceCube(&o->objects[k]->bb), frustumPlanes)){

			*ret = (Object **)realloc(*ret, sizeof(Object *) * ++(*nObjects));
			(*ret)[*nObjects-1] = o->objects[k];
		}
	}

	if(o->children[0])
		for(k = 0; k < 8; k++)
			FindViewCollisions(o->children[k], ret, nObjects, frustumPlanes);
}

void OctreeLeaf_GetVisibleObjects(OctreeLeaf *octant, Object ***ret, int *nObjects){

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

	FindViewCollisions(octant, ret, nObjects, frustumPlanes);
}
