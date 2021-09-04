#ifndef SKYBOX_DEF
#define SKYBOX_DEF

#include "math.h"
#include "image_loader.h"

typedef struct {
	Image img;
	float size;
	Vec3 pos;
	unsigned int vao;
	unsigned int posVbo;
	unsigned int uvVbo;
} Skybox;

Skybox Skybox_Create(float size, Vec3 pos, const char *texturePath);
void Skybox_Draw(Skybox *skybox);
void Skybox_Free(Skybox *skybox);

#endif