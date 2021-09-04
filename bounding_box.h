#ifndef BOUNDING_BOX_DEF
#define BOUNDING_BOX_DEF

#include "math.h"

typedef struct _BoundingBox {
	struct _BoundingBox *parent;
	struct _BoundingBox *children;
	short numChildren;
	Cube cube;
	Vec3 pos;
	char **types;
	int nTypes;
} BoundingBox;

typedef struct Object Object;

int BoundingBox_CheckCollision(BoundingBox *bb, BoundingBox *bb2);

void BoundingBox_AddType(BoundingBox *bb, char *name);

Cube BoundingBox_GetWorldSpaceCube(BoundingBox *bb);

BoundingBox BoundingBox_Create(Cube cube, Vec3 pos);

int BoundingBox_AddChild(BoundingBox *bb, BoundingBox *child);

void BoundingBox_FreeData(BoundingBox *bb);

void BoundingBox_Copy(BoundingBox *into, BoundingBox *bb);

int BoundingBox_ResolveCollision(Object *obj1, BoundingBox *bb, Object *obj2, BoundingBox *bb2);

Vec3 BoundingBox_GetPosition(BoundingBox *bb);

float BoundingBox_CheckCollisionRay(BoundingBox *bb, Ray ray, BoundingBox **closest);

BoundingBox *BoundingBox_GetTop(BoundingBox *bb);

#endif
