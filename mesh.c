#include <GL/glew.h>
#include "mesh.h"
#include "world.h"
#include "hash_table.h"
#include "shaders.h"
#include "skeletal_animation.h"
#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Vec3 position;
    Vec2 uv;
    Vec3 normal;
} PackedVertex;

typedef struct {
    Vec3 *vecs;
    int nVecs;
} Vec3Arr;

typedef struct {
    Vec2 *vecs;
    int nVecs;
} Vec2Arr;

static void PushVec3Arr(Vec3Arr *arr, Vec3 push){
    arr->vecs = (Vec3 *)realloc(arr->vecs, sizeof(Vec3) * ++arr->nVecs);
    arr->vecs[arr->nVecs-1] = push;
}

static void PushVec2Arr(Vec2Arr *arr, Vec2 push){
    arr->vecs = (Vec2 *)realloc(arr->vecs, sizeof(Vec2) * ++arr->nVecs);
    arr->vecs[arr->nVecs-1] = push;
}

static Vec2Arr CreateVec2Arr(){
    Vec2Arr ret;
    memset(&ret, 0, sizeof(Vec2Arr));
    return ret;
}

static Vec3Arr CreateVec3Arr(){
    Vec3Arr ret;
    memset(&ret, 0, sizeof(Vec3Arr));
    return ret;
}

static void ReadImage(FILE *fp, const char *fileName, Image *image){
    char path[512];
    fscanf(fp, "%s", path);

    int k;
    for(k = strlen(fileName); k > 0; k --)
        if(fileName[k] == '/') break;

    char fullPath[512] = {};

    memcpy(fullPath, fileName, k+1);
    strcat(fullPath, path);

    *image = ImageLoader_CreateImage(fullPath, 1);

    // ImageLoader_SetImageParameters(image, GL_NEAREST,GL_NEAREST,GL_REPEAT,GL_REPEAT);
    // ImageLoader_SetImageParameters(*image, GL_LINEAR,GL_LINEAR,GL_REPEAT,GL_REPEAT);
}

static int qSortMaterials(const void *a, const void *b){
    if(((Material *)a)->startingElementIndex > ((Material *)b)->startingElementIndex) return 1;
    if(((Material *)a)->startingElementIndex < ((Material *)b)->startingElementIndex) return -1;
    return 0;
}

void Mesh_Free(Mesh *mesh){

    int k;
    for(k = 0; k < mesh->nMaterials; k++)
        ImageLoader_DeleteImage(&mesh->materials[k].diffuseMapImage);

    if(mesh->materials) free(mesh->materials);

    if(mesh->verts) free(mesh->verts);
    if(mesh->coords) free(mesh->coords);
    if(mesh->norms) free(mesh->norms);
    if(mesh->weights) free(mesh->weights);
    if(mesh->boneIndices) free(mesh->boneIndices);
    if(mesh->elements) free(mesh->elements);

    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->posVbo);
    glDeleteBuffers(1, &mesh->uvVbo);
    glDeleteBuffers(1, &mesh->normVbo);
    glDeleteBuffers(1, &mesh->boneWeightVbo);
    glDeleteBuffers(1, &mesh->boneIndexVbo);
}

int Mesh_Load(Mesh *mesh, char *fileName, char loadUvs, char loadNorms, RiggedMesh *riggedMesh){

	FILE *fp;
	fp = fopen(fileName, "r");
    if(fp == NULL) { printf("Error loading model: %s\n", fileName); return 0; }

    char lineType[512] = {};

    Vec3Arr tempBoneWeights = CreateVec3Arr();
    Vec3Arr tempBoneIndices = CreateVec3Arr();
    Vec3Arr tempVerts = CreateVec3Arr();
    Vec3Arr tempNorms = CreateVec3Arr();
    Vec2Arr tempUvs = CreateVec2Arr();
    Vec3Arr verts = CreateVec3Arr();
    Vec3Arr norms = CreateVec3Arr();
    Vec2Arr coords = CreateVec2Arr();
    Vec3Arr boneWeights = CreateVec3Arr();
    Vec3Arr boneIndices = CreateVec3Arr();

    memset(mesh, 0, sizeof(Mesh));

    while(!feof(fp)){
        fscanf( fp , "%s ", lineType );

        if(strcmp(lineType, "tex") == 0){

            Material mat;
            ReadImage(fp, fileName, &mat.diffuseMapImage);

            mesh->materials = (Material *)realloc(mesh->materials, sizeof(Material) * ++mesh->nMaterials);
            mesh->materials[mesh->nMaterials-1] = mat;

        } else if( strcmp(lineType, "texIndex" ) == 0 ){

            int index = 0;
            fscanf(fp, "%i", &index);

            if(index >= 0 && index < (int)mesh->nMaterials){
                mesh->materials[index].startingElementIndex = verts.nVecs;
                mesh->materials[index].texIndex = index;
            }

        } else if( strcmp(lineType, "v" ) == 0 ){

            Vec3 vert;
            fscanf( fp,"%f %f %f", &vert.x, &vert.y, &vert.z);

            PushVec3Arr(&tempVerts, vert);

            if(riggedMesh){
                PushVec3Arr(&tempBoneIndices, (Vec3){-1,-1,-1});
                PushVec3Arr(&tempBoneWeights, (Vec3){0,0,0});
            }

        } else if( strcmp(lineType, "vw" ) == 0 ){
            if(riggedMesh){

                char name[64];
                float weight = 0;
                fscanf( fp,"%s : ", name);
                fscanf(fp, "%f", &weight);

                int boneIndex = RiggedMesh_GetBoneFromName(riggedMesh, name)->index;
                if(boneIndex != -1){

                    float *bi = &tempBoneIndices.vecs[tempBoneIndices.nVecs-1].x;
                    float *bw = &tempBoneWeights.vecs[tempBoneWeights.nVecs-1].x;

                    int k;
                    for(k = 0; k < 3 && bi[k] != -1; k++){}

                    if(k == 3) {

                        int smallestIndex = -1;
                        float smallest = 1;

                        for(k = 0; k < 3; k++){
                            if(bw[k] < smallest){
                                smallest = bw[k];
                                smallestIndex = k;
                            }
                        }

                        if(weight > bw[smallestIndex]){
                            bi[smallestIndex] = boneIndex;
                            bw[smallestIndex] = weight;
                        }

                    } else {
                        bi[k] = boneIndex;
                        bw[k] = weight;
                    }
                }
            }

        } else if( strcmp(lineType, "vt" ) == 0 && loadUvs){
            Vec2 texCoord;
            fscanf( fp,"%f %f", &texCoord.x, &texCoord.y );

            PushVec2Arr(&tempUvs, texCoord);

        } else if( strcmp(lineType, "vn" ) == 0 && loadNorms){
            Vec3 norm;
            fscanf( fp,"%f %f %f", &norm.x, &norm.y, &norm.z );

            PushVec3Arr(&tempNorms, norm);

        } else if( strcmp(lineType, "f" ) == 0 ){
            int v[3] = {}, u[3] = {}, n[3] = {};

            if(loadUvs && loadNorms)
                fscanf( fp,"%i/%i/%i %i/%i/%i %i/%i/%i", &v[0], &u[0], &n[0], &v[1], &u[1], &n[1], &v[2], &u[2], &n[2]);
            else if(loadUvs)
                fscanf( fp,"%i/%i/ %i/%i/ %i/%i/", &v[0], &u[0], &v[1], &u[1], &v[2], &u[2]);
            else if(loadNorms)
                fscanf( fp,"%i//%i %i//%i %i//%i", &v[0], &n[0], &v[1], &n[1], &v[2], &n[2]);
            else
                fscanf( fp,"%i// %i// %i//", &v[0], &v[1], &v[2]);

            int f;
            for(f = 0; f < 3; f++) if(v[f] > 0 && v[f] - 1 < (int)tempVerts.nVecs) PushVec3Arr(&verts, tempVerts.vecs[v[f] - 1]);
            for(f = 0; f < 3; f++) if(u[f] > 0 && u[f] - 1 < (int)tempUvs.nVecs)   PushVec2Arr(&coords, tempUvs.vecs[u[f] - 1]);
            for(f = 0; f < 3; f++) if(n[f] > 0 && n[f] - 1 < (int)tempNorms.nVecs) PushVec3Arr(&norms, tempNorms.vecs[n[f] - 1]);

            if(riggedMesh){
                for(f = 0; f < 3; f++) if(v[f] > 0 && v[f] - 1 < (int)tempBoneIndices.nVecs) PushVec3Arr(&boneIndices, tempBoneIndices.vecs[v[f] - 1]);
                for(f = 0; f < 3; f++) if(v[f] > 0 && v[f] - 1 < (int)tempBoneWeights.nVecs) PushVec3Arr(&boneWeights, tempBoneWeights.vecs[v[f] - 1]);
            }
        }

        while( !feof(fp) && fgetc(fp) != '\n' ) {}
    }

    fclose(fp);

    int topElementIndex = 0;
    
    HashTable vertMap = HashTable_Create(verts.nVecs);

    int k;
    for(k = 0; k < (int)verts.nVecs; k++){

        PackedVertex packed = {verts.vecs[k], (Vec2){0,0}, (Vec3){0,0,0}};

        if(loadUvs && k < (int)coords.nVecs) packed.uv = coords.vecs[k];
        if(loadNorms && k < (int)norms.nVecs) packed.normal = norms.vecs[k];

        int *index = (int *)HashTable_Get(&vertMap, (uint8_t *)&packed, sizeof(PackedVertex));

        if(index){

            mesh->elements = (short *)realloc(mesh->elements,sizeof(short) * ++mesh->nElements);
            mesh->elements[mesh->nElements-1] = *index;

        } else {

            mesh->verts = (Vec3 *)realloc(mesh->verts,sizeof(Vec3) * ++mesh->nVerts);
            mesh->verts[mesh->nVerts-1] = verts.vecs[k];

            if(loadNorms && k < (int)norms.nVecs){
            
                mesh->norms = (Vec3 *)realloc(mesh->norms,sizeof(Vec3) * ++mesh->nNorms);
                mesh->norms[mesh->nNorms-1] = norms.vecs[k];
            }

            if(loadUvs && k < (int)coords.nVecs){

                mesh->coords = (Vec2 *)realloc(mesh->coords,sizeof(Vec2) * ++mesh->nCoords);
                mesh->coords[mesh->nCoords-1] = coords.vecs[k];
            }

            if(riggedMesh){

                mesh->weights = (Vec3 *)realloc(mesh->weights,sizeof(Vec3) * ++mesh->nWeights);
                mesh->weights[mesh->nWeights-1] = boneWeights.vecs[k];

                mesh->boneIndices = (Vec3 *)realloc(mesh->boneIndices,sizeof(Vec3) * ++mesh->nBoneIndices);
                mesh->boneIndices[mesh->nBoneIndices-1] = boneIndices.vecs[k];
            }

            mesh->elements = (short *)realloc(mesh->elements,sizeof(short) * ++mesh->nElements);
            mesh->elements[mesh->nElements-1] = topElementIndex;

            HashTable_Add(&vertMap, (uint8_t *)&packed, sizeof(PackedVertex), (uint8_t *)&topElementIndex, sizeof(int));

            topElementIndex++;
        }
    }

    HashTable_Free(&vertMap);

    qsort(mesh->materials, mesh->nMaterials, sizeof(Material), qSortMaterials);

    Vec3 modelSpace = (Vec3){HUGE_VAL,HUGE_VAL,HUGE_VAL};
    float width = -HUGE_VAL, height = -HUGE_VAL, depth = -HUGE_VAL;

    for(int k = 0; k < (int)mesh->nVerts; k++){
        if(mesh->verts[k].x < modelSpace.x)          modelSpace.x = mesh->verts[k].x;
        if(mesh->verts[k].y < modelSpace.y)          modelSpace.y = mesh->verts[k].y;
        if(mesh->verts[k].z < modelSpace.z)          modelSpace.z = mesh->verts[k].z;
        if(mesh->verts[k].x > modelSpace.x+width)    width  = mesh->verts[k].x-modelSpace.x;
        if(mesh->verts[k].y > modelSpace.y+height)   height = mesh->verts[k].y-modelSpace.y;
        if(mesh->verts[k].z > modelSpace.z+depth)    depth  = mesh->verts[k].z-modelSpace.z;
    }

    mesh->minCube = Math_Cube(modelSpace, width, height, depth);

    return 1;
}

void Mesh_InitializeDrawing(Mesh *mesh,int shader){

    Shaders_UseProgram(shader);

    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->posVbo);
    glGenBuffers(1, &mesh->normVbo);
    glGenBuffers(1, &mesh->uvVbo);

    if(mesh->nWeights){
        glGenBuffers(1, &mesh->boneIndexVbo);
        glGenBuffers(1, &mesh->boneWeightVbo);
    }

    mesh->shader = shader;

    GLuint positionAttribute = glGetAttribLocation(Shaders_GetProgram(shader), SHADERS_POSITION_ATTRIB);
    GLuint uvAttribute = glGetAttribLocation(Shaders_GetProgram(shader), SHADERS_COORD_ATTRIB);
    GLuint normAttribute = glGetAttribLocation(Shaders_GetProgram(shader), SHADERS_NORM_ATTRIB);

    glEnableVertexAttribArray(positionAttribute);
    glEnableVertexAttribArray(uvAttribute);
    glEnableVertexAttribArray(normAttribute);

    glBindBuffer(GL_ARRAY_BUFFER,mesh->posVbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->nVerts*sizeof(Vec3), &mesh->verts[0].x, GL_STATIC_DRAW);
    glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);

    if(mesh->nCoords){
        glBindBuffer(GL_ARRAY_BUFFER,mesh->uvVbo);
        glBufferData(GL_ARRAY_BUFFER,mesh->nCoords*sizeof(Vec2), &mesh->coords[0].x, GL_STATIC_DRAW);
        glVertexAttribPointer(uvAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    if(mesh->nNorms){
        glBindBuffer(GL_ARRAY_BUFFER,mesh->normVbo);
        glBufferData(GL_ARRAY_BUFFER,mesh->nNorms*sizeof(Vec3), &mesh->norms[0].x, GL_STATIC_DRAW);
        glVertexAttribPointer( normAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    if(mesh->nWeights){
        int indexAttribute = glGetAttribLocation(Shaders_GetProgram(SKELETAL_ANIMATION_SHADER), "boneIndices");
        int weightsAttribute = glGetAttribLocation(Shaders_GetProgram(SKELETAL_ANIMATION_SHADER), "boneWeights");

        glEnableVertexAttribArray(indexAttribute);
        glEnableVertexAttribArray(weightsAttribute);

        glBindBuffer(GL_ARRAY_BUFFER,mesh->boneWeightVbo);
        glVertexAttribPointer( weightsAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBufferData(GL_ARRAY_BUFFER,mesh->nWeights*sizeof(Vec3), &mesh->weights[0].x, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER,mesh->boneIndexVbo);
        glVertexAttribPointer( indexAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBufferData(GL_ARRAY_BUFFER,mesh->nBoneIndices*sizeof(Vec3), &mesh->boneIndices[0].x, GL_STATIC_DRAW);
    }

    glBindVertexArray(0);
}

void Mesh_DrawElements(Mesh *mesh){

    // printf("%i\n",mesh->nElements/3 ); // tri count
    Shaders_UseProgram(mesh->shader);

    Shaders_UpdateProjectionMatrix();
    Shaders_UpdateViewMatrix();
    Shaders_UpdateModelMatrix();

    glActiveTexture(GL_TEXTURE0);

    if(!mesh->nMaterials){

        glDrawElements(GL_TRIANGLES, mesh->nElements, GL_UNSIGNED_SHORT, &mesh->elements[0]);

    } else {

        for(int k = 0; k < (int)mesh->nMaterials; k++){

            int size = 0;

            if(k == (int)mesh->nMaterials-1)
                size = mesh->nElements - mesh->materials[k].startingElementIndex;
            else
                size = mesh->materials[k+1].startingElementIndex - mesh->materials[k].startingElementIndex;

            unsigned int tex = mesh->materials[k].diffuseMapImage.glTexture;

            // if(overrideMaterials != nullptr){
            //     for(int m = 0; m < (int)overrideMaterials->size(); m++){
            //         if(overrideMaterials->at(m).texIndex == k){
            //             tex = overrideMaterials->at(m).diffuseMapImage.GetTexture();
            //             break;
            //         }
            //     }
            // }

            glBindTexture(GL_TEXTURE_2D, tex);
            // printf("%i %i %i %i\n", mesh->materials[k].diffuseMapImage.GetTexture(), mesh->materials[k].startingElementIndex, size, mesh->modelElementsSize);

            glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_SHORT, &mesh->elements[mesh->materials[k].startingElementIndex]);
        }
    }
}

void Mesh_Draw(Mesh *mesh){
    glBindVertexArray(mesh->vao);
    Mesh_DrawElements(mesh);
}

static void DrawMeshInstance(Object *obj){
    Mesh *mesh = (Mesh *)obj->data;
    Mesh_Draw(mesh);
}

Object *MeshInstance_Create(Mesh *mesh){

    Object *obj = Object_Create();

    obj->data = malloc(sizeof(Mesh));

    memcpy(obj->data, mesh, sizeof(Mesh));

    obj->bb = BoundingBox_Create(mesh->minCube, (Vec3){0,0,0});

    obj->Draw = DrawMeshInstance;
    
    World_UpdateObjectInOctree(obj);

    return obj;
}