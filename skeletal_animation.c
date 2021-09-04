#define FRAME_RATE 30
#define GLEW_STATIC
#include <GL/glew.h>
#include "skeletal_animation.h"
#include "window.h"
#include "shaders.h"
#include "world.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Bone *RiggedMesh_FindBoneFromIndex(Bone *b, int index){
	
	if(!b) return NULL;

	if(b->index == index)
		return b;

	int k;
	for(k = 0; k < (int)b->nChildren; k++){
		Bone *bone = NULL;
		bone = RiggedMesh_FindBoneFromIndex(b->children[k], index);
		if(bone) return bone;
	}

	return NULL;
}

Bone *RiggedMesh_FindBoneFromName(Bone *b, const char *name){
	
	if(strcmp(b->name, name) == 0)
		return b;
	
	int k;
	for(k = 0; k < (int)b->nChildren; k++){
		Bone *bone = NULL;
		bone = RiggedMesh_FindBoneFromName(b->children[k], name);
		if(bone) return bone;
	}

	return NULL;
}

static void GetFramePositions(Bone *b, Animation *anim, int *lastRotFrame, int *nextRotFrame, float timeInto){
	int k;
	for(k = 0; k < (int)anim->nKeyframes; k++){
		if(anim->keyframes[k].boneIndex == b->index){
			if(anim->keyframes[k].pos*(1000/FRAME_RATE) <= timeInto){
				*lastRotFrame = k;
			} else {
				*nextRotFrame = k; 
				break;
			}
		}
	}
}

static Quat GetCurrentBoneRotation(Bone *b, Animation *anim, PlayingAnimation *pAnim, int *frameChange, int *doSlerp){
	
	if(!b) return (Quat){0,0,0,1};
	
	int timeInto = Window_GetTicks() - pAnim->timeStarted; 
	int lastRotFrame = -1, nextRotFrame = -1;
	GetFramePositions(b, anim, &lastRotFrame, &nextRotFrame, timeInto);

	*doSlerp = 0;
	*frameChange = 1;

	if(lastRotFrame == -1){
		if(nextRotFrame == -1) return (Quat){0,0,0,1};
		return anim->keyframes[nextRotFrame].rotation;
	}

	int maxPos = 0;
	for(int k = 0; k < (int)anim->nKeyframes; k++)
		if(anim->keyframes[k].pos > maxPos)
			maxPos = anim->keyframes[k].pos;

	if(anim->keyframes[lastRotFrame].pos >= maxPos){
		
		if(!pAnim->loop) {

			pAnim->ended = 1;

		} else {

			pAnim->timeStarted = Window_GetTicks();
			nextRotFrame = lastRotFrame = -1;
			timeInto = *doSlerp = 0;
			GetFramePositions(b, anim, &lastRotFrame, &nextRotFrame, timeInto);
		}
	}

	if(lastRotFrame == -1){
		if(nextRotFrame == -1) return (Quat){1,0,0,0};
		return anim->keyframes[nextRotFrame].rotation;
	}

	if(nextRotFrame == -1)
		return anim->keyframes[lastRotFrame].rotation;

	float animTime = fabs(1000/FRAME_RATE)*(anim->keyframes[nextRotFrame].pos - anim->keyframes[lastRotFrame].pos);
	float t = fabs((float)timeInto - ((1000/FRAME_RATE)*anim->keyframes[lastRotFrame].pos)) / animTime;

	if(t > 1) t = 1;
	if(t < 0) t = 0;

	Quat qa = anim->keyframes[lastRotFrame].rotation;
	Quat qb = anim->keyframes[nextRotFrame].rotation;

	*frameChange = *doSlerp = 1;

	return Math_Slerp(qa, qb, t);
}

static Vec3 getCurrentBoneScale(Bone *b, Animation *anim, PlayingAnimation *pAnim){
	
	if(!b) return (Vec3){1,1,1};
	
	int timeInto = Window_GetTicks() - pAnim->timeStarted; 
	int lastScaleFrame = -1, nextScaleFrame = -1;
	GetFramePositions(b, anim, &lastScaleFrame, &nextScaleFrame, timeInto);

	if(lastScaleFrame == -1) return b->scale;
	if(nextScaleFrame == -1) return anim->keyframes[lastScaleFrame].scale;

	int maxPos = 0;
	for(int k = 0; k < (int)anim->nKeyframes; k++)
		if(anim->keyframes[k].pos > maxPos)
			maxPos = anim->keyframes[k].pos;

	if(anim->keyframes[lastScaleFrame].pos >= maxPos){
		nextScaleFrame = 0;
		pAnim->timeStarted = Window_GetTicks();
		timeInto = 0;
		GetFramePositions(b, anim, &lastScaleFrame, &nextScaleFrame, timeInto);
	}

	float animTime = fabs(1000/FRAME_RATE)*(anim->keyframes[nextScaleFrame].pos - anim->keyframes[lastScaleFrame].pos);
	float t = fabs((float)timeInto - ((1000/FRAME_RATE)*anim->keyframes[lastScaleFrame].pos)) / animTime;

	if(t > 1) t = 1;
	if(t < 0) t = 0;

	return Math_LerpVec3(anim->keyframes[lastScaleFrame].scale, anim->keyframes[nextScaleFrame].scale, t);
}

static Vec3 getCurrentBonePos(Bone *b, Animation *anim, PlayingAnimation *pAnim){
	
	if(!b) return (Vec3){0,0,0};
	
	int timeInto = Window_GetTicks() - pAnim->timeStarted; 
	int lastPosFrame = -1, nextPosFrame = -1;
	GetFramePositions(b, anim, &lastPosFrame, &nextPosFrame, timeInto);

	if(lastPosFrame == -1) return b->pos;
	if(nextPosFrame == -1) return anim->keyframes[lastPosFrame].position;

	int maxPos = 0;
	for(int k = 0; k < (int)anim->nKeyframes; k++)
		if(anim->keyframes[k].pos > maxPos)
			maxPos = anim->keyframes[k].pos;

	if(anim->keyframes[lastPosFrame].pos >= maxPos){
		nextPosFrame = 0;
		pAnim->timeStarted = Window_GetTicks();
		timeInto = 0;
		GetFramePositions(b, anim, &lastPosFrame, &nextPosFrame, timeInto);
	}

	float animTime = fabs(1000/FRAME_RATE)*(anim->keyframes[nextPosFrame].pos - anim->keyframes[lastPosFrame].pos);
	float t = fabs((float)timeInto - ((1000/FRAME_RATE)*anim->keyframes[lastPosFrame].pos)) / animTime;

	if(t > 1) t = 1;
	if(t < 0) t = 0;

	return Math_LerpVec3(anim->keyframes[lastPosFrame].position, anim->keyframes[nextPosFrame].position, t);
}

static int BoneUpdate(Bone *b, Animation *anims, int nAnims, PlayingAnimation *pAnims, int nPAnims, float *mats){
	
	if(!b) return 1;

	// char rotSet = false;
	Quat rot = b->rot;

	// if(b->startSlerpTime != -1){
		
	// 	float t = (Window_GetTicks() - b->startSlerpTime) * b->slerpSpeed;
		
	// 	if(t > 1.0)
	// 		b->startSlerpTime = -1;
	// 	else
	// 		rot = Math_Slerp(b->currRot, b->rot, t);
	// }

	Vec3 scale = b->scale;
	Vec3 pos = b->pos;

	int k;
	for(k = 0; k < nPAnims; k++){

		int rotTempFrameChange = 0, doSlerp = 0;
		
		rot = Math_QuatMult(rot, GetCurrentBoneRotation(b, &anims[k], &pAnims[k], &rotTempFrameChange, &doSlerp));

		// scale = getCurrentBoneScale(b, &anims[it->first], &pAnims[it->first]);
		// pos = getCurrentBonePos(b, &anims[it->first], &pAnims[it->first]);

		// if(rotTempFrameChange) {

		// 	if(doSlerp && rotSet)
		// 		rot = Math_Slerp(rot, rot2, it->second.weight);
		// 	else
		// 		rot = rot2;
			
		// 	rotSet = 1;
		// }
	}

	float rotMatrix[16]; 
	float finalMatrix[16];
	float transMatrix[16] = {
		scale.x,0,0,pos.x,
		0,scale.y,0,pos.y,
		0,0,scale.z,pos.z,
		0,0,0,1
	};

	if(b->overrideRotation){
		Math_CopyMatrix(rotMatrix ,b->rotMatrix);
	} else {
		Math_MatrixFromQuat(rot, rotMatrix);
		b->currRot = rot;
	}

	Math_MatrixMatrixMult(b->matrix, transMatrix, rotMatrix);

	if(b->parent){
		Math_MatrixMatrixMult(b->absMatrix,  b->parent->absMatrix, b->matrix);
		Math_MatrixMatrixMult(b->absRotationMatrix,  b->parent->absRotationMatrix, rotMatrix);
	} else {
		Math_CopyMatrix(b->absMatrix, b->matrix);
		Math_CopyMatrix(b->absRotationMatrix, rotMatrix);
	}

	if(b->parent && b->overrideRotation){

		float invRot[16];
		Math_CopyMatrix(invRot, b->absRotationMatrix);
		Math_InverseMatrix(invRot);
		Math_MatrixMatrixMult(b->absMatrix, b->absMatrix, invRot);
		Math_MatrixMatrixMult(b->absMatrix, b->absMatrix, rotMatrix);
		Math_CopyMatrix(b->absRotationMatrix, rotMatrix);
	}

	Math_MatrixMatrixMult(finalMatrix, b->absMatrix, b->invBindMatrix);

	Math_CopyMatrix(&mats[b->index*16], finalMatrix);
	
	for(int j = 0; j < (int)b->nChildren; j++)
		BoneUpdate(b->children[j], anims, nAnims, pAnims, nPAnims, mats);

	return 0;
}

static void GetBonesStartingPositions(Bone *b){
	
	if(!b) return;

	float rotMatrix[16] = {0}; 
	float finalMatrix[16] = {0};
	float transMatrix[16] = {
		b->scale.x,0,0,b->pos.x,
		0,b->scale.y,0,b->pos.y,
		0,0,b->scale.z,b->pos.z,
		0,0,0,1
	};

	Math_MatrixFromQuat(b->startingRot, rotMatrix);

	Math_MatrixMatrixMult(b->matrix, transMatrix, rotMatrix);

	if(b->parent)
		Math_MatrixMatrixMult(b->absMatrix,  b->parent->absMatrix, b->matrix);
	else
		Math_CopyMatrix(b->absMatrix, b->matrix);

	Math_MatrixMatrixMult(finalMatrix, b->absMatrix, b->invBindMatrix);

	Vec3 point = {0,0,0};
	b->startingPos = Math_MatrixMult(point, b->absMatrix);

	for(int j = 0; j < (int)b->nChildren; j++)
		GetBonesStartingPositions(b->children[j]);
}

static void LoadSkel(RiggedMesh *rMesh, char *skelPath){

	FILE *fp = fopen(skelPath, "rb");
	if(!fp) { printf("Error loading skeleton %s\n", skelPath); return; }

	int index = 0;
	while(!feof(fp)){

		rMesh->numOfBones++;

	    int parent_index;

	    Bone *temp = (Bone *)malloc(sizeof(Bone));
	    memset(temp, 0, sizeof(Bone));

	    char name[64];
	    memset(name, 0, sizeof(name));

		int res = fscanf(fp, "%s %i %f %f %f %f %f %f %f %f %f %f\n", name, &parent_index, &temp->pos.x, &temp->pos.y, &temp->pos.z, 
			&temp->rot.x, &temp->rot.y, &temp->rot.z, &temp->rot.w, &temp->scale.x, &temp->scale.y, &temp->scale.z);

		temp->startingRot = temp->rot;
		strcpy(temp->name, name);

		if(res == 12){

	    	temp->index = index;

	    	if(parent_index == -1){

	    		rMesh->rootBone = temp;

	    	} else {

	    		if(rMesh->rootBone != NULL){

			    	Bone *parent = NULL;
			    	parent = RiggedMesh_FindBoneFromIndex(rMesh->rootBone, parent_index);
			    	if(parent){
				    	temp->parent = parent;
				    	parent->children = (Bone **)realloc(parent->children, ++parent->nChildren * sizeof(Bone *));
				    	parent->children[parent->nChildren-1] = temp;
			    	}
			    }
	    	}

	    	index++;

	    } else {

			while(fgetc(fp) != '\n' && !feof(fp)){}
			continue;
	    }

		float transMatrix[16] = {
			temp->scale.x,0,0,temp->pos.x,
			0,temp->scale.y,0,temp->pos.y,
			0,0,temp->scale.z,temp->pos.z,
			0,0,0,1
		};

		float rotMatrix[16];
		Math_MatrixFromQuat(temp->rot, rotMatrix );
		Math_MatrixMatrixMult(temp->invBindMatrix, transMatrix, rotMatrix);

		if(temp->parent){
			Math_MatrixMatrixMult(temp->absMatrix,  temp->parent->absMatrix, temp->invBindMatrix);
			temp->startingAbsRot = Math_QuatMult(temp->parent->startingAbsRot, Math_MatrixToQuat(rotMatrix));
		} else {
			Math_CopyMatrix(temp->absMatrix, temp->invBindMatrix);
			temp->startingAbsRot = Math_MatrixToQuat(rotMatrix);
		}
 
		Math_CopyMatrix(temp->invBindMatrix, temp->absMatrix);
		Math_InverseMatrix(temp->invBindMatrix);
	}

	fclose(fp);

	GetBonesStartingPositions(rMesh->rootBone);
}

void RiggedMesh_Load(RiggedMesh *rMesh, char *meshPath, char *skelPath){

	memset(rMesh, 0, sizeof(RiggedMesh));

	LoadSkel(rMesh, skelPath);

	Mesh_Load(&rMesh->mesh, meshPath, 1, 1, rMesh);
		
	Mesh_InitializeDrawing(&rMesh->mesh, SKELETAL_ANIMATION_SHADER);
}

void RiggedMesh_LoadAnim(RiggedMesh *rMesh, const char *animPath, const char *name){
	
    FILE *fp = fopen(animPath, "rb");
    if(!fp) { printf("Error loading animation: %s\n", animPath); return; }

    rMesh->animations = (Animation *)realloc(rMesh->animations, sizeof(Animation) * ++rMesh->nAnimations);
	
	Animation *added = &rMesh->animations[rMesh->nAnimations-1];
	memset(added, 0, sizeof(Animation));

	strcpy(added->name, name);

    while(!feof(fp)){

    	char type;
    	fscanf(fp, "%c ", &type);

    	if(type == 'a'){

	    	Keyframe tmp;

			fscanf(fp, "%i %i %f %f %f %f %f %f %f %f %f %f\n", &tmp.pos, &tmp.boneIndex, &tmp.position.x, 
				&tmp.position.y, &tmp.position.z, &tmp.rotation.x, &tmp.rotation.y, 
				&tmp.rotation.z,&tmp.rotation.w, &tmp.scale.x, &tmp.scale.y, 
				&tmp.scale.z);

	    	added->keyframes = (Keyframe *)realloc(added->keyframes, sizeof(Keyframe) * ++added->nKeyframes);
	    	added->keyframes[added->nKeyframes-1] = tmp;
    	}

    	else if(type == 't'){

   //  		int index;
	  //   	char path[512];
			// fscanf(fp, "%i %s\n", &index, path);

		 //    int k;
		 //    for(k = (int)strlen(animPath); k > 0; k --)
		 //        if(animPath[k] == '/') break;

		 //    char fullPath[512] = {};

		 //    memcpy(fullPath, animPath, k+1);
		 //    strcat(fullPath, path);

		 //    Material mat;

		 //    mat.texIndex = index;

		 //    mat.diffuseMapImage = ImageLoader_CreateImage(fullPath, 1);
		 //    ImageLoader_SetImageParameters(mat.diffuseMapImage, GL_NEAREST,GL_NEAREST,GL_REPEAT,GL_REPEAT);
    		
   //  		added->overrideMaterials = (Material *)realloc(added->overrideMaterials, ++added->nOverrideMaterials * sizeof(Material));
			// added->overrideMaterials[added->nOverrideMaterials-1] = mat;
    	}
    }

    fclose(fp);
}

int RiggedMesh_GetBoneIndexFromName(RiggedMesh *rMesh, const char *name){
	Bone *b = RiggedMesh_FindBoneFromName(rMesh->rootBone, name);
	if(b == NULL) return -1;
	return b->index;
}

Bone *RiggedMesh_GetBoneFromName(RiggedMesh *rMesh, const char *name){
	return RiggedMesh_FindBoneFromName(rMesh->rootBone, name);
}

int RiggedMeshInstance_GetBoneIndexFromName(RiggedMesh *rMesh, const char *name){
	Bone *b = RiggedMesh_FindBoneFromName(rMesh->rootBone, name);
	if(b == NULL) return -1;
	return b->index;
}

Bone *RiggedMeshInstance_GetBoneFromName(RiggedMesh *rMesh, const char *name){
	return RiggedMesh_FindBoneFromName(rMesh->rootBone, name);
}

void RiggedMeshInstance_Draw(RiggedMeshInstance *rMeshInstance){
	if(!rMeshInstance->nPlayingAnims || !rMeshInstance->nShaderMatricies) return;
	Shaders_UseProgram(SKELETAL_ANIMATION_SHADER);
	glUniformMatrix4fv(Shaders_GetBonesLocation(), rMeshInstance->nShaderMatricies, GL_TRUE, rMeshInstance->shaderMatricies);
	// Shaders_SetModelMatrix(GetModelMatrix());
	// Shaders_UpdateModelMatrix();
	Mesh_Draw(&rMeshInstance->rMesh->mesh);
}


int RiggedMeshInstance_Update(RiggedMeshInstance *rMeshInstance){
	if(!rMeshInstance->nPlayingAnims || !rMeshInstance->nShaderMatricies) return 0;

	BoneUpdate(rMeshInstance->rootBone, rMeshInstance->rMesh->animations, rMeshInstance->rMesh->nAnimations,
		rMeshInstance->playingAnims, rMeshInstance->nPlayingAnims, rMeshInstance->shaderMatricies);

	return 1;
}

RiggedMeshInstance RiggedMeshInstance_Create(RiggedMesh *m){

	RiggedMeshInstance ret;
	memset(&ret, 0, sizeof(ret));

	ret.rMesh = m;

	ret.nShaderMatricies = m->numOfBones;
	ret.shaderMatricies = (float *)malloc(sizeof(float) * 16 * ret.nShaderMatricies);

	ret.rootBone = RiggedMesh_CopyBones(m);

	return ret;
}

void RiggedMeshInstance_StartAnimation(RiggedMeshInstance *rMeshInstance, char *name, float weight, char loop){

	rMeshInstance->playingAnims = (PlayingAnimation *)realloc(rMeshInstance->playingAnims,
		sizeof(PlayingAnimation) * ++rMeshInstance->nPlayingAnims);

	PlayingAnimation *pAnim = &rMeshInstance->playingAnims[rMeshInstance->nPlayingAnims-1];
	
	memset(pAnim, 0, sizeof(PlayingAnimation));

	pAnim->timeStarted = Window_GetTicks();
	pAnim->weight = weight;
	pAnim->loop = loop;
	pAnim->ended = 0;
	strcpy(pAnim->name, name);
}

void RiggedMeshInstance_StopAnimation(RiggedMeshInstance *rMeshInstance, char *name){

	int k;
	for(k = 0; k < rMeshInstance->nPlayingAnims; k++){
		if(strcmp(rMeshInstance->playingAnims[k].name, name) == 0){

			int j;
			for(j = k; j < rMeshInstance->nPlayingAnims-1; j++)
				rMeshInstance->playingAnims[j] = rMeshInstance->playingAnims[j+1];

			rMeshInstance->playingAnims = (PlayingAnimation *)realloc(rMeshInstance->playingAnims,
				sizeof(PlayingAnimation) * --rMeshInstance->nPlayingAnims);
		
			break;
		}
	}
}

void Bone_Free(Bone *bone){

	int k;
	for(k = 0; k < bone->nChildren; k++)
		Bone_Free(bone->children[k]);

	free(bone);
}

void Animation_Free(Animation *anim){
	free(anim->keyframes);
}

void RiggedMesh_Free(RiggedMesh *riggedMesh){
	
	Bone_Free(riggedMesh->rootBone);

	int k;
	for(k = 0; k < riggedMesh->nAnimations; k++)
		Animation_Free(&riggedMesh->animations[k]);

	free(riggedMesh->animations);

	Mesh_Free(&riggedMesh->mesh);
}

void RiggedMeshInstance_Free(RiggedMeshInstance *rMeshInstance){

	if(rMeshInstance->shaderMatricies) free(rMeshInstance->shaderMatricies);

	Bone_Free(rMeshInstance->rootBone);
}

void RiggedMeshInstance_StopAllAnimations(RiggedMeshInstance *rMeshInstance){

	free(rMeshInstance->playingAnims);
	rMeshInstance->nPlayingAnims = 0;
}

static Bone *CopyBone(Bone *bone, Bone *parent){

	Bone *ret = (Bone *)malloc(sizeof(Bone));
	
	*ret = *bone;

	ret->nChildren = 0;
	ret->children = NULL;

	ret->parent = parent;

	for(int k = 0; k < (int)bone->nChildren; k++){
		ret->children = (Bone **)realloc(ret->children, sizeof(Bone *) * ++ret->nChildren);
		ret->children[ret->nChildren-1] = CopyBone(bone->children[k], ret);
	}

	return ret;
}

Bone *RiggedMesh_CopyBones(RiggedMesh *rMesh){

	return CopyBone(rMesh->rootBone, NULL);
}

static void DrawRiggedMeshObject(Object *obj){
    RiggedMeshInstance *rMesh = (RiggedMeshInstance *)obj->data;
    RiggedMeshInstance_Draw(rMesh);
}

Object *RiggedMeshObject_Create(RiggedMeshInstance *rMeshInstance){

    Object *obj = Object_Create();

    obj->data = malloc(sizeof(RiggedMeshInstance));

    memcpy(obj->data, rMeshInstance, sizeof(RiggedMeshInstance));

    obj->bb = BoundingBox_Create(rMeshInstance->rMesh->mesh.minCube, (Vec3){0,0,0});

    obj->Draw = DrawRiggedMeshObject;
    
    World_UpdateObjectInOctree(obj);

    return obj;
}