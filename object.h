#ifndef OBJECT_DEF
#define OBJECT_DEF

#include "bounding_box.h"
#include "image_loader.h"
#include "math.h"
#include <stdio.h>

typedef struct OctreeLeaf OctreeLeaf;

typedef struct {
	Object *obj1;
	Object *obj2;
	BoundingBox *bb1;
	BoundingBox *bb2;
} Collision;

struct Object {

	void (*Free)(Object *this);
	void (*AddUser)(Object *this);
	void (*RemoveUser)(Object *this);
	void (*OnCollision)(Object *this, Object *obj2, BoundingBox *bb1, BoundingBox *bb2);
	void (*Update)(Object *this);
	void (*Draw)(Object *this);
	void (*SetPosition)(Object *this, Vec3 pos);
	void (*Rotate)(Object *this, Vec3 rot);
	void (*Scale)(Object *this, Vec3 scale);
	void (*UpdateMatrix)(Object *this);
	Object *(*Copy)(Object *this);


	float matrix[16];
	float rotation[16];
	float translation[16];
	float scale[16];

	int shader;

	BoundingBox bb;

	char frozen;
	
	char offScreenUpdated;

	OctreeLeaf *inOctant;

	void *data;

	char storeLastCollisions;
	Collision *lastCollisions;
	int nLastCollisions;

	int numUsers; // always remember to inc
};

Object *Object_Create();
Object *Object_Copy(Object *obj);
void Object_Free(Object *obj);
void Object_Freeze(Object *obj);
Object *Object_Set(Object *ret);

#endif