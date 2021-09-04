#ifndef WORLD_DEF
#define WORLD_DEF

#include "math.h"
#include "object.h"
#include "bounding_box.h"

void World_DrawCube(Cube r);
Object *World_GetVisibleObjects();
void World_DrawOctree();
void World_DrawBoundingBox(BoundingBox *box);
void World_InitOctree(Vec3 pos, float size, float octantWidth);
float World_GetWidth();
float World_GetHeight();
float World_GetDepth();
void World_UpdateObjects();
void World_UpdateObjectInOctree(Object *obj);
void World_RemoveObjectFromOctree(Object *box);
void World_DrawLines(Vec3 *lines, int num);
void World_Render(char drawBoundingBoxes);
void World_ResolveCollisions(Object *obj, BoundingBox *box);
Vec3 World_GetTopLeft();
void World_Free();
void World_Clear();
void World_DrawObject(Object *object);
void World_AddOffScreenUpdatedObject(Object *obj);
void World_RemoveOffScreenUpdatedObject(Object *obj);

#endif