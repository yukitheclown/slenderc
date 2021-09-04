#ifndef MESH_DEF
#define MESH_DEF

#include "image_loader.h"
#include "object.h"
#include "math.h"

typedef struct RiggedMesh RiggedMesh;

typedef struct {
	int startingElementIndex;
    Image diffuseMapImage;
    char name[128];
    int texIndex;
} Material;

typedef struct {
	Vec3 *verts;
	Vec3 *norms;
	Vec2 *coords;
	Vec3 *weights;
	Vec3 *boneIndices;
	short *elements;
	int nVerts;
	int nNorms;
	int nCoords;
	int nWeights;
	int nBoneIndices;
	int nElements;
	Material *materials;
	int nMaterials;
	Cube minCube;
	unsigned int vao;
	unsigned int posVbo;
	unsigned int uvVbo;
	unsigned int normVbo;
	unsigned int boneWeightVbo;
	unsigned int boneIndexVbo;
	int shader;
} Mesh;

void Mesh_Free(Mesh *mesh);
int Mesh_Load(Mesh *mesh, char *fileName, char loadUvs, char loadNormals, RiggedMesh *riggedMesh);
void Mesh_Draw(Mesh *mesh);
void Mesh_InitializeDrawing(Mesh *mesh,int shader);
Object *MeshInstance_Create(Mesh *mesh);

#endif