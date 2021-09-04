#include "bounding_box.h"
#include "window.h"
#include "object.h"
#include <stdio.h>
#include <stdlib.h>
#include "math.h"

void BoundingBox_Copy(BoundingBox *into, BoundingBox *bb){

	memcpy(into, bb, sizeof(BoundingBox));

	into->children = NULL;

	into->children = (BoundingBox *)malloc(bb->numChildren*sizeof(BoundingBox));
	into->types = (char **)malloc(bb->nTypes*sizeof(char *));

	int k;
	for(k = 0; k < bb->nTypes; k++){
		
		into->types[k] = malloc(strlen(bb->types[k]) + 1);
		strcpy(into->types[k], bb->types[k]);
		into->types[k][strlen(bb->types[k])] = 0;
	}

	for(k = 0; k < bb->numChildren; k++){
		BoundingBox_Copy(&into->children[k], &bb->children[k]);
		into->children[k].parent = into;
	}
}

void BoundingBox_AddType(BoundingBox *bb, char *name){
	
	if(!strlen(name)) return;

	bb->types = (char **)realloc(bb->types, sizeof(char *) * ++bb->nTypes);
	bb->types[bb->nTypes-1] = malloc(strlen(name) + 1);
	memset(bb->types[bb->nTypes-1], 0, strlen(name) + 1);
	strcpy(bb->types[bb->nTypes-1], name);
}

void BoundingBox_FreeData(BoundingBox *bb){
	int k;

	if(bb->types){

		for(k = 0; k < bb->nTypes; k++)
			free(bb->types[k]);
		
		free(bb->types);
	}

	if(bb->children){
		
		for(k = 0; k < bb->numChildren; k++)
			BoundingBox_FreeData(&bb->children[k]);
		
		free(bb->children);
	}

	memset(bb, 0, sizeof(BoundingBox));

	if(bb->parent){
		
		for(k = 0; k < bb->parent->numChildren; k++){
			if(&bb->parent->children[k] == bb){
				
				int j;
				for(j = k; j < bb->parent->numChildren-1; j++)
					bb->parent->children[j] = bb->parent->children[j+1];
			
				bb->parent->children = (BoundingBox *)realloc(bb->parent->children, sizeof(BoundingBox) * --bb->parent->numChildren);
			
				break;
			}

		}
	}
}

Vec3 BoundingBox_GetPosition(BoundingBox *bb){

	if(bb->parent)
		return Math_Vec3AddVec3(BoundingBox_GetPosition(bb->parent), bb->pos);

	return bb->pos;
}

BoundingBox *BoundingBox_GetTop(BoundingBox *bb){

	if(bb->parent)
		return BoundingBox_GetTop(bb->parent);

	return bb;
}

BoundingBox BoundingBox_Create(Cube cube, Vec3 pos){

	BoundingBox bb;
	memset(&bb, 0, sizeof(BoundingBox));

	bb.cube = cube;
	bb.pos = pos;
	
	return bb;
}

Cube BoundingBox_GetWorldSpaceCube(BoundingBox *bb){
	return Math_Cube(Math_Vec3AddVec3(Math_CubeXYZ(bb->cube), BoundingBox_GetPosition(bb)), bb->cube.w, bb->cube.h, bb->cube.d);
}

int BoundingBox_AddChild(BoundingBox *bb, BoundingBox *child) {

	int index = bb->numChildren;

	bb->children = (BoundingBox *)realloc(bb->children, (++bb->numChildren)*sizeof(BoundingBox));

	BoundingBox_Copy(&bb->children[index], child);
	bb->children[index].parent = bb;

	return index;
}

int BoundingBox_CheckCollision(BoundingBox *bb, BoundingBox *bb2){


	int ret = 0;

	int k;
    for(k = 0; k < bb->numChildren; k++)
    	ret += BoundingBox_CheckCollision(&bb->children[k], bb2);

    for(k = 0; k < bb2->numChildren; k++)
    	ret += BoundingBox_CheckCollision(bb, &bb2->children[k]);

	if(!Math_CheckCollisionCube(BoundingBox_GetWorldSpaceCube(bb), BoundingBox_GetWorldSpaceCube(bb2))) return ret;

	return ret+1;
}

int BoundingBox_ResolveCollision(Object *obj1, BoundingBox *bb, Object *obj2, BoundingBox *bb2){


	int ret = 0;

	int k;
    for(k = 0; k < bb->numChildren; k++)
    	ret += BoundingBox_ResolveCollision(obj1, &bb->children[k], obj2, bb2);

    for(k = 0; k < bb2->numChildren; k++)
    	ret += BoundingBox_ResolveCollision(obj1, bb, obj2, &bb2->children[k]);

	if(!Math_CheckCollisionCube(BoundingBox_GetWorldSpaceCube(bb), BoundingBox_GetWorldSpaceCube(bb2))) return ret;

	if(obj1 && (obj1->storeLastCollisions || obj1->OnCollision)){
		
		if(obj1->OnCollision) obj1->OnCollision(obj1, obj2, bb, bb2);
		
		if(obj1->storeLastCollisions){
			obj1->lastCollisions = (Collision *)realloc(obj1->lastCollisions, sizeof(Collision) * ++obj1->nLastCollisions);
			obj1->lastCollisions[obj1->nLastCollisions-1] = (Collision){obj1, obj2, bb, bb2};
		}
	}

	if(obj2 && obj2->storeLastCollisions){

		if(obj2->storeLastCollisions){
			obj2->lastCollisions = (Collision *)realloc(obj2->lastCollisions, sizeof(Collision) * ++obj2->nLastCollisions);
			obj2->lastCollisions[obj2->nLastCollisions-1] = (Collision){obj2, obj1, bb2, bb};
		}
	}

    return ret+1;
}


float BoundingBox_CheckCollisionRay(BoundingBox *bb, Ray ray, BoundingBox **closest){

	float ret = HUGE_VAL;

	int k;

	for(k = 0; k < bb->numChildren; k++){

		BoundingBox *tBb;

		float d = BoundingBox_CheckCollisionRay(&bb->children[k], ray, &tBb);
	
		if(d < ret){
			*closest = tBb;
			ret = d;
		}
	}

	return Math_CubeCheckCollisionRay(BoundingBox_GetWorldSpaceCube(bb), ray);
}