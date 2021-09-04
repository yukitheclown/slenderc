#ifndef SKELETAL_DEF
#define SKELETAL_DEF

#include "math.h"
#include "mesh.h"

typedef struct Bone Bone;

struct Bone {
	Vec3 pos;
	Vec3 startingPos;
	Quat rot;
	Quat startingRot;
	Quat currRot;
	Vec3 scale;
	char name[64];
	Bone *parent;
	Bone **children;
	int nChildren;
	float invBindMatrix[16];
	float matrix[16];
	float absMatrix[16];
	float absRotationMatrix[16];
	Quat startingAbsRot;
	float rotMatrix[16];
	int index;
	char isHidden;
	char overrideRotation;
	int startSlerpTime;
	float slerpSpeed;
};

typedef struct {
	Vec3 position;
	Quat rotation;
	Vec3 scale;
	int pos;
	int boneIndex;
} Keyframe;

typedef struct {
	Keyframe *keyframes;
	int nKeyframes;
	char name[128];
} Animation;

typedef struct {
	int timeStarted;
	float weight;
	char loop;
	char ended;
	char name[128];
} PlayingAnimation;

struct RiggedMesh {
	Mesh mesh;
	int numOfBones;
	Animation *animations;
	int nAnimations;
	Bone *rootBone;
};

typedef struct {

	RiggedMesh *rMesh;

	float *shaderMatricies;
	int nShaderMatricies;

	PlayingAnimation *playingAnims;
	int nPlayingAnims;

	Bone *rootBone;

} RiggedMeshInstance;

void RiggedMesh_Load(RiggedMesh *rMesh, char *meshPath, char *skelPath);
void RiggedMesh_LoadAnim(RiggedMesh *rMesh, const char *path, const char *name);

Bone *RiggedMesh_GetBoneFromName(RiggedMesh *rMesh, const char *name);

RiggedMeshInstance RiggedMeshInstance_Create(RiggedMesh *m);

void RiggedMeshInstance_Draw(RiggedMeshInstance *rMesh);

Bone *RiggedMesh_CopyBones(RiggedMesh *rMesh);

void Animation_Free(Animation *anim);
void Bone_Free(Bone *b);
void RiggedMesh_Free(RiggedMesh *riggedMesh);
void RiggedMeshInstance_Free(RiggedMeshInstance *riggedMesh);

Object *RiggedMeshObject_Create(RiggedMeshInstance *rMeshInstance);

void RiggedMeshInstance_StopAnimation(RiggedMeshInstance *rMeshInstance, char *name);
void RiggedMeshInstance_StartAnimation(RiggedMeshInstance *rMeshInstance, char *name, float weight, char loop);
int RiggedMeshInstance_Update(RiggedMeshInstance *rMeshInstance);

#endif