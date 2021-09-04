#include <GL/glew.h>
#include "window.h"
#include "object.h"
#include "sound.h"
#include "world.h"
#include "skeletal_animation.h"
#include "mesh.h"
#include "image_loader.h"
#include "text.h"
#include "shaders.h"
#include "text.h"
// #include "skybox.h"

#define NOTE_RANGE 1.5

static Sound ambienceSound;
static Sound footstepsSound;
static Sound noteSound;
static Sound songSound;
static Sound flashlightSound;

static Object *notes[3];
static int collectedNotes = 0;
static int flashlightOn = 0;

static int endingTriggered = 0;

static RiggedMeshInstance progressManInstance;

// static Skybox skybox;

static float mouseSensitivity = 0.001;
static float moveSpeed = 0.005;

static Vec2 rotation = {0,0};
static Vec3 position = {0,1.8,8};

static char movingDirs[4];

static void SetObjectsPos(Object *obj, Vec3 pos, Vec3 rotation);

static void Update(){

	RiggedMeshInstance_Update(&progressManInstance);

	Vec3 moveVec = {0,0,0};

	if(movingDirs[0]) moveVec.z -= 1;
	if(movingDirs[1]) moveVec.z += 1;
	if(movingDirs[2]) moveVec.x -= 1;
	if(movingDirs[3]) moveVec.x += 1;
    
    if(Math_Vec3Magnitude(moveVec)){

        moveVec = Math_Vec3Normalize(moveVec);

        moveVec = Math_Rotate(moveVec, (Vec3){-rotation.y, -rotation.x, 0});

        moveVec = Math_Vec3MultFloat(moveVec, Window_GetDeltaTime() * moveSpeed);

        position.x += moveVec.x;
        position.z += moveVec.z;

    	Object *player = Object_Create();
        player->bb = BoundingBox_Create((Cube){-0.5,-0.1,-0.5,1,0.2,1}, position);

        player->storeLastCollisions = 1;

        World_ResolveCollisions(player, &player->bb);

        int k;
        for(k = 0; k < player->nLastCollisions; k++){

        	int j;
    	    for(j = 0; j < player->lastCollisions[k].bb2->nTypes; j++){
    	    	if(strcmp(player->lastCollisions[k].bb2->types[j], "None") == 0)
    	    		break;
    	    }

    	    if(j != player->lastCollisions[k].bb2->nTypes) continue;

        	Cube c1 = BoundingBox_GetWorldSpaceCube(player->lastCollisions[k].bb1);
        	Cube c2 = BoundingBox_GetWorldSpaceCube(player->lastCollisions[k].bb2);
        	Vec3 dist = Math_GetDistanceCube(c1, c2);

        	float *distArr = &dist.x;

    		Vec3 norms[] = { {1,0,0}, {0,1,0}, {0,0,1} };
        	
    		float smallest = HUGE_VAL;
    		Vec3 ourOverlap = {0,0,0};
        
        	for(j = 0; j < 3; j++){

        		if(fabs(distArr[j]) < smallest){
        			
        			smallest = fabs(distArr[j]);

    	    		ourOverlap = Math_Vec3MultFloat(norms[j], distArr[j]);
        		}
        	}
        
            position.x -= ourOverlap.x;
            position.z -= ourOverlap.z;
        }


        player->Free(player);
    }
}

static void Event(SDL_Event ev){


	if(ev.type == SDL_MOUSEMOTION){

		rotation.x += mouseSensitivity * ev.motion.xrel;
		rotation.y += mouseSensitivity * ev.motion.yrel;

        while(rotation.x > PI*2) rotation.x -= PI*2;
        while(rotation.x < 0) rotation.x += PI*2;

        if(rotation.y < -PI/2.5) rotation.y = -PI/2.5;
        if(rotation.y > PI/3) rotation.y = PI/3;

	} else if(ev.type == SDL_KEYDOWN){

		if(ev.key.keysym.sym == SDLK_w)
			movingDirs[0] = 1;
		else if(ev.key.keysym.sym == SDLK_s)
			movingDirs[1] = 1;
		else if(ev.key.keysym.sym == SDLK_a)
			movingDirs[2] = 1;
        else if(ev.key.keysym.sym == SDLK_d)
            movingDirs[3] = 1;
        else if(ev.key.keysym.sym == SDLK_f){
            flashlightOn = !flashlightOn;
            Sound_Play(flashlightSound, 0);
            Shaders_UseProgram(TEXTURED_SHADER);
            Shaders_SetUseFlashlight(flashlightOn);
            Shaders_UseProgram(SKELETAL_ANIMATION_SHADER);
            Shaders_SetUseFlashlight(flashlightOn);
        }

        Sound_Play(footstepsSound, 1);

	} else if(ev.type == SDL_KEYUP){

		if(ev.key.keysym.sym == SDLK_w)
			movingDirs[0] = 0;
		else if(ev.key.keysym.sym == SDLK_s)
			movingDirs[1] = 0;
		else if(ev.key.keysym.sym == SDLK_a)
			movingDirs[2] = 0;
		else if(ev.key.keysym.sym == SDLK_d)
			movingDirs[3] = 0;
		else if(ev.key.keysym.sym == SDLK_e){

			int k;
			for(k = 0; k < (int)(sizeof(notes)/sizeof(Object *)); k++){

				if(notes[k] == NULL) continue;

				Vec3 bbPos = (Vec3){ notes[k]->bb.pos.x, position.y, notes[k]->bb.pos.z };

				if(Math_Vec3Magnitude(Math_Vec3SubVec3(bbPos, position)) < NOTE_RANGE){
                    // SetObjectsPos(RiggedMeshObject_Create(&progressManInstance), (Vec3){-3,0,0}, (Vec3){0,3.14,0});
                    Sound_Play(noteSound, 0);
                    Sound_SetSoundsPosition(noteSound, bbPos);
					collectedNotes++;
					World_RemoveObjectFromOctree(notes[k]);
					notes[k] = NULL;
				}
			}
		}

        int k;
        for(k = 0; k < 4; k++) if(movingDirs[k] != 0) break;

        if(k == 4) Sound_Stop(footstepsSound);
	}

    Sound_SetPosition(position, Math_Rotate((Vec3){0,0,-1}, (Vec3){-rotation.y, -rotation.x, 0}));
    Sound_SetSoundsPosition(ambienceSound, position);
    Sound_SetSoundsPosition(flashlightSound, position);
    Sound_SetSoundsPosition(footstepsSound, position);
    Sound_SetSoundsPosition(songSound, position);
}

static void Focus(){

}

static char Draw(){
    
    Vec3 forward = Math_Rotate((Vec3){0,0,-1}, (Vec3){-rotation.y, -rotation.x, 0});

    float view[16];
    Math_LookAt(view, position, Math_Vec3AddVec3(position, forward), (Vec3){0,1,0});
    Shaders_SetViewMatrix(view);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(endingTriggered){

        static int lastSwitchTime = 0;
        static int onColor = -1;

        Vec4 colors[] = {
            {1,0.5,0.5,1},
            {0.5,1,0.5,1},
            {0.5,0.5,1,1},
        };

        if(lastSwitchTime == 0){
            lastSwitchTime = (Window_GetTicks() - (60000 / 128)) + 20;
        }

        if(Window_GetTicks() - lastSwitchTime > 60000 / 128){
            onColor = (onColor+1) % (int)(sizeof(colors)/sizeof(Vec4));
            lastSwitchTime = Window_GetTicks();
        }

        Shaders_UseProgram(TEXTURED_SHADER);
        Shaders_SetUniformColor(colors[onColor]);
        Shaders_UseProgram(SKELETAL_ANIMATION_SHADER);
        Shaders_SetUniformColor(colors[onColor]);
    }

	// Skybox_Draw(&skybox);

	// World_DrawOctree();

	World_Render(0);

    return 1;
}

static void OnResize(){

}

static void SetObjectsPos(Object *obj, Vec3 pos, Vec3 rotation){

    obj->SetPosition(obj, pos);
    obj->Rotate(obj, rotation);
    obj->UpdateMatrix(obj);

    obj->AddUser(obj);
    World_UpdateObjectInOctree(obj);
    obj->RemoveUser(obj);
}

int main(int argc, char **argv){

	// names for vw im mesh loading are so inneficient.

    Window_Open("Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480*2, 272*2, 0);

    memset(movingDirs, 0, sizeof(movingDirs));

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_DITHER);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    Shaders_Init();
    Text_Init();
    Sound_Init();

    ambienceSound = Sound_CreateSound("Resources/ambience.ogg", (Vec3){0,0,0});
    footstepsSound = Sound_CreateSound("Resources/footsteps.ogg", (Vec3){0,0,0});
    noteSound = Sound_CreateSound("Resources/note.ogg", (Vec3){0,0,0});
    songSound = Sound_CreateSound("Resources/Architecture In Tokyo - SILK.ogg", (Vec3){0,0,0});
    flashlightSound = Sound_CreateSound("Resources/flashlight.ogg", (Vec3){0,0,0});

    Sound_Play(ambienceSound, 1);

    World_InitOctree((Vec3){-100, -100, -100}, 200, 25);

    glClearColor(0,0,0,1);

    float persp[16], view[16], model[16];
    Math_Perspective(persp, 60.0f*(3.1415/180), (float)1920 / (float)1080, 0.1f, 50.0f);
    Math_LookAt(view, (Vec3){8,0,8}, (Vec3){0,0,0}, (Vec3){0,1,0});
    
    Math_Identity(model);

    Shaders_SetProjectionMatrix(persp);
    Shaders_SetModelMatrix(model);
    Shaders_SetViewMatrix(view);

    // skybox = Skybox_Create(500, (Vec3){0,0,0}, "Resources/skybox.png");

    Mesh noteMesh;
    Mesh_Load(&noteMesh, "Resources/note.yuk", 1, 1, NULL);
    Mesh_InitializeDrawing(&noteMesh, TEXTURED_SHADER);

    notes[0] = MeshInstance_Create(&noteMesh);
    BoundingBox_AddType(&notes[0]->bb, "None");

    SetObjectsPos(notes[0], (Vec3){-2.5,-1.3,-3.4}, (Vec3){0,0,0});

    notes[1] = MeshInstance_Create(&noteMesh);
    BoundingBox_AddType(&notes[1]->bb, "None");

    SetObjectsPos(notes[1], (Vec3){12,-1.3,49.85}, (Vec3){0,0,0});

    notes[2] = MeshInstance_Create(&noteMesh);
    BoundingBox_AddType(&notes[2]->bb, "None");

    SetObjectsPos(notes[2], (Vec3){2,-1.3,8.05 - 60}, (Vec3){0,0,0});

    Mesh bathroomMesh;
    Mesh_Load(&bathroomMesh, "Resources/bathroom.yuk", 1, 1, NULL);
    Mesh_InitializeDrawing(&bathroomMesh, TEXTURED_SHADER);

    Object *bathroom = MeshInstance_Create(&bathroomMesh);
    BoundingBox_AddType(&bathroom->bb, "None");

    BoundingBox bathroomBbChildren[] = {
        BoundingBox_Create((Cube){-5.54, 0.03, -3.21, 0.1, 4.18, 6.6}, (Vec3){0,0,0}),
        BoundingBox_Create((Cube){-5.54 + 10.9, 0.03, -3.21, 0.1, 4.18, 6.6}, (Vec3){0,0,0}),
        BoundingBox_Create((Cube){-5.54, 0.03, -3.21, 11, 4.18, 0.1}, (Vec3){0,0,0}),
        BoundingBox_Create((Cube){-5.54, 0.03, -3.21+6.55, 7.5, 4.18, 0.1}, (Vec3){0,0,0}),
        BoundingBox_Create((Cube){3.8, 0.03, -3.21+6.55, 1.6, 4.18, 0.1}, (Vec3){0,0,0}),
        BoundingBox_Create((Cube){-5.54+3.5, 0.03, -3.21, 0.1, 4.18, 2.7}, (Vec3){0,0,0}),
        BoundingBox_Create((Cube){-5.54+6, 0.03, -3.21, 0.1, 4.18, 2.7}, (Vec3){0,0,0}),
    };

    int k;
    for(k = 0; k < (int)(sizeof(bathroomBbChildren)/sizeof(BoundingBox)); k++)
	    BoundingBox_AddChild(&bathroom->bb, &bathroomBbChildren[k]);

    bathroom->AddUser(bathroom);
    World_UpdateObjectInOctree(bathroom);
    bathroom->RemoveUser(bathroom);

    Mesh wallsMesh;
    Mesh_Load(&wallsMesh, "Resources/walls.yuk", 1, 1, NULL);
    Mesh_InitializeDrawing(&wallsMesh, TEXTURED_SHADER);

    Object *walls = MeshInstance_Create(&wallsMesh);
    BoundingBox_AddType(&walls->bb, "None");

    SetObjectsPos(walls, (Vec3){10,0,50}, (Vec3){0,0,0});

    BoundingBox wallsBbChildren[] = {
        BoundingBox_Create((Cube){-3.18, -0.31, -0.5, 6.14, 4.91, 0.8}, (Vec3){0,0,0}),
        BoundingBox_Create((Cube){-0.5, -0.31, -3.11, 0.8, 4.91, 5.93}, (Vec3){0,0,0}),
        BoundingBox_Create((Cube){-3.18, -0.31, 0, 2, 4.91, 2.5}, (Vec3){0,0,0}),
    };

    for(k = 0; k < (int)(sizeof(wallsBbChildren)/sizeof(BoundingBox)); k++)
        BoundingBox_AddChild(&walls->bb, &wallsBbChildren[k]);

    walls->AddUser(walls);
    World_UpdateObjectInOctree(walls);
    walls->RemoveUser(walls);


    Mesh tunnelMesh;
    Mesh_Load(&tunnelMesh, "Resources/tunnel.yuk", 1, 1, NULL);
    Mesh_InitializeDrawing(&tunnelMesh, TEXTURED_SHADER);

    Object *tunnel = MeshInstance_Create(&tunnelMesh);
    BoundingBox_AddType(&tunnel->bb, "None");

    SetObjectsPos(tunnel, (Vec3){0,0,-50}, (Vec3){0,0,0});

    BoundingBox tunnelBbChildren[] = {
        BoundingBox_Create((Cube){-6.1, -0.98, -5.08, 11.2, 4.43, 3.8}, (Vec3){0,0,0}),
        BoundingBox_Create((Cube){-5.8, -0.98, 1.1, 11, 4.43, 3.5}, (Vec3){0,0,0}),

    };

    for(k = 0; k < (int)(sizeof(tunnelBbChildren)/sizeof(BoundingBox)); k++)
        BoundingBox_AddChild(&tunnel->bb, &tunnelBbChildren[k]);

    tunnel->AddUser(tunnel);
    World_UpdateObjectInOctree(tunnel);
    tunnel->RemoveUser(tunnel);


	RiggedMesh progressMan;
	RiggedMesh_Load(&progressMan, "Resources/progressman.yuk", "Resources/progressman.skl");
	RiggedMesh_LoadAnim(&progressMan, "Resources/progressman_Armature.001Action.anm", "wave");

	progressManInstance = RiggedMeshInstance_Create(&progressMan);

	RiggedMeshInstance_StartAnimation(&progressManInstance, "wave", 1, 1);

    SetObjectsPos(RiggedMeshObject_Create(&progressManInstance), (Vec3){-10,0,11}, (Vec3){0,0,0});

    Sound_SetSoundsPosition(songSound, position);

    // Sound_Play(songSound, 1);

    Mesh groundMesh;
    Mesh_Load(&groundMesh, "Resources/ground.yuk", 1, 1, NULL);
    Mesh_InitializeDrawing(&groundMesh, TEXTURED_SHADER);

    MeshInstance_Create(&groundMesh);

    Mesh rocksMeshes[2];
    Mesh_Load(&rocksMeshes[0], "Resources/rocks2.yuk", 1, 1, NULL);
    Mesh_InitializeDrawing(&rocksMeshes[0], TEXTURED_SHADER);
    Mesh_Load(&rocksMeshes[1], "Resources/rocks1.yuk", 1, 1, NULL);
    Mesh_InitializeDrawing(&rocksMeshes[1], TEXTURED_SHADER);

    Vec3 rockPositions[] = {
    	{0,0,-80},
    	{0,0,80},
    	{80,0,0},
    	{-80,0,0},
    };

    for(k = 0; k < 4; k++)
	    SetObjectsPos(MeshInstance_Create(&rocksMeshes[k/2]), rockPositions[k], (Vec3){0,0,0});

    Mesh treeMeshes[4];

    Mesh_Load(&treeMeshes[0], "Resources/tree0.yuk", 1, 1, NULL);
    Mesh_InitializeDrawing(&treeMeshes[0], TEXTURED_SHADER);

    Mesh_Load(&treeMeshes[1], "Resources/tree1.yuk", 1, 1, NULL);
    Mesh_InitializeDrawing(&treeMeshes[1], TEXTURED_SHADER);

    Mesh_Load(&treeMeshes[2], "Resources/tree2.yuk", 1, 1, NULL);
    Mesh_InitializeDrawing(&treeMeshes[2], TEXTURED_SHADER);

    Mesh_Load(&treeMeshes[3], "Resources/tree3.yuk", 1, 1, NULL);
    Mesh_InitializeDrawing(&treeMeshes[3], TEXTURED_SHADER);

    Cube treeCube = (Cube){0,0,-0.1,0.2,5,0.2};
    BoundingBox treeBb = BoundingBox_Create(treeCube,(Vec3){0,0,0});

    Vec3 treePositions[300];

    for(k = 0; k < 300; k++){
    	treePositions[k].x = -50 + (rand()%10000)/100;
    	treePositions[k].z = -50 + (rand()%10000)/100;
    }

    for(k = 0; k < (int)(sizeof(treePositions)/sizeof(Vec3)); k++){

	    Object *obj = MeshInstance_Create(&treeMeshes[rand()%4]);

	    BoundingBox_AddType(&obj->bb, "None");

	    BoundingBox_AddChild(&obj->bb, &treeBb);

	    SetObjectsPos(obj, treePositions[k], (Vec3){0,0,0});
    }

    // Sound_Play(songSound, 1);
    // endingTriggered = 1;

    Window_MainLoop(Update, Event, Draw, Focus, OnResize, 1, 1);

 	// Skybox_Free(&skybox);
    Mesh_Free(&bathroomMesh);
    Mesh_Free(&tunnelMesh);
    Mesh_Free(&groundMesh);
    Mesh_Free(&noteMesh);
    Mesh_Free(&wallsMesh);
    RiggedMesh_Free(&progressMan);
    RiggedMeshInstance_Free(&progressManInstance);

    Sound_FreeSound(ambienceSound);
    Sound_FreeSound(flashlightSound);
    Sound_FreeSound(footstepsSound);
    Sound_FreeSound(noteSound);

    for(k = 0; k < (int)(sizeof(treeMeshes)/sizeof(Mesh)); k++)
	    Mesh_Free(&treeMeshes[k]);

    for(k = 0; k < (int)(sizeof(rocksMeshes)/sizeof(Mesh)); k++)
	    Mesh_Free(&rocksMeshes[k]);

	World_Free();
    Text_Close();
    Shaders_Close();
    ImageLoader_Free();
    Sound_Close();


    return 0;
}