#define GLEW_STATIC
#include <GL/glew.h>
#include "skybox.h"
#include "shaders.h"
#include "mesh.h"
#include "math.h"
#include "shaders.h"
#include "image_loader.h"

void Skybox_Draw(Skybox *skybox){
	
	glDepthMask(GL_FALSE);

	Shaders_UseProgram(TEXTURED_SHADER);

    Shaders_UpdateProjectionMatrix();
    Shaders_UpdateViewMatrix();
    Shaders_UpdateModelMatrix();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, skybox->img.glTexture);

	float model[16];
	Math_TranslateMatrix(model, skybox->pos);
	Shaders_SetModelMatrix(model);
	Shaders_UpdateModelMatrix();

	glBindVertexArray(skybox->vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);

	glDepthMask(GL_TRUE);
}

Skybox Skybox_Create(float size, Vec3 pos, const char *texturePath){

	Skybox ret;
	memset(&ret, 0, sizeof(ret));
	ret.pos = pos;

	ret.img = ImageLoader_CreateImage(texturePath, 1);

	Shaders_UseProgram(TEXTURED_SHADER);

	glGenVertexArrays(1, &ret.vao);
	glBindVertexArray(ret.vao);

	glGenBuffers(1, &ret.posVbo);
	glGenBuffers(1, &ret.uvVbo);

    int posAttrib = glGetAttribLocation(Shaders_GetProgram(TEXTURED_SHADER), SHADERS_POSITION_ATTRIB);
    int uvAttrib = glGetAttribLocation(Shaders_GetProgram(TEXTURED_SHADER), SHADERS_COORD_ATTRIB);

	Vec3 verts[36] = {
	    { -0.5f, -0.5f, -0.5f }, 
	    {  0.5f, -0.5f, -0.5f }, 
	    {  0.5f,  0.5f, -0.5f }, 
	    {  0.5f,  0.5f, -0.5f }, 
	    { -0.5f,  0.5f, -0.5f }, 
	    { -0.5f, -0.5f, -0.5f }, 

	    { -0.5f, -0.5f,  0.5f }, 
	    {  0.5f, -0.5f,  0.5f }, 
	    {  0.5f,  0.5f,  0.5f }, 
	    {  0.5f,  0.5f,  0.5f }, 
	    { -0.5f,  0.5f,  0.5f }, 
	    { -0.5f, -0.5f,  0.5f }, 

	    { -0.5f,  0.5f,  0.5f }, 
	    { -0.5f,  0.5f, -0.5f }, 
	    { -0.5f, -0.5f, -0.5f }, 
	    { -0.5f, -0.5f, -0.5f }, 
	    { -0.5f, -0.5f,  0.5f }, 
	    { -0.5f,  0.5f,  0.5f }, 

	    {  0.5f,  0.5f,  0.5f }, 
	    {  0.5f,  0.5f, -0.5f }, 
	    {  0.5f, -0.5f, -0.5f }, 
	    {  0.5f, -0.5f, -0.5f }, 
	    {  0.5f, -0.5f,  0.5f }, 
	    {  0.5f,  0.5f,  0.5f }, 

	    { -0.5f, -0.5f, -0.5f }, 
	    {  0.5f, -0.5f, -0.5f }, 
	    {  0.5f, -0.5f,  0.5f }, 
	    {  0.5f, -0.5f,  0.5f }, 
	    { -0.5f, -0.5f,  0.5f }, 
	    { -0.5f, -0.5f, -0.5f }, 

	    { -0.5f,  0.5f, -0.5f }, 
	    {  0.5f,  0.5f, -0.5f }, 
	    {  0.5f,  0.5f,  0.5f }, 
	    {  0.5f,  0.5f,  0.5f }, 
	    { -0.5f,  0.5f,  0.5f }, 
	    { -0.5f,  0.5f, -0.5f }, 
	};

	float th = 1.0/3.0, tw = 1.0/4.0;
	float oh = 1.0/ret.img.h, ow = 1.0/ret.img.w;

	float w0 = tw - (ow*2), w1 = tw - (ow*3), w2 = tw - (ow*4), w3 = tw - (ow*5);
	float h0 = th - (oh*2), h1 = th - (oh*3), h2 = th - (oh*4);
	float x0 = ow, x1 = tw + (ow*2), x2 = (tw*2) + (ow*3), x3 = (tw*3) + (ow*4);
	float y0 = oh, y1 = th + (oh*2), y2 = (th*2) + (oh*3);

	Vec2 coords[36] = {
		{ x1,   	y1}, //Back one
		{ x1+w1, 	y1},
		{ x1+w1, 	y1+h1},
		{ x1+w1, 	y1+h1},
		{ x1,   	y1+h1},
		{ x1,   	y1},
		{ x3+w3, 	y1}, //Front one
		{ x3, 		y1},
		{ x3, 		y1+h1},
		{ x3, 		y1+h1},
		{ x3+w3, 	y1+h1},
		{ x3+w3, 	y1},
		{ x0,   	y1+h1}, //Left one
		{ x0+w0, 	y1+h1},
		{ x0+w0, 	y1},
		{ x0+w0, 	y1},
		{ x0,   	y1},
		{ x0,   	y1+h1},
		{ x2+w2, 	y1+h1}, //Right one
		{ x2, 		y1+h1},
		{ x2, 		y1},
		{ x2, 		y1},
		{ x2+w2, 	y1},
		{ x2+w2, 	y1+h1},
		{ x1,   	y0+h0}, //Bottom one
		{ x1+w1, 	y0+h0},
		{ x1+w1, 	y0},
		{ x1+w1, 	y0},
		{ x1,   	y0},
		{ x1,   	y0+h0},
		{ x1,   	y2}, //Top one
		{ x1+w1, 	y2},
		{ x1+w1, 	y2+h2},
		{ x1+w1, 	y2+h2},
		{ x1,   	y2+h2},
		{ x1,   	y2},
	};

	for(int k = 0; k < 36; k++)
		verts[k] = Math_Vec3MultFloat(verts[k], size);

	glBindVertexArray(ret.vao);

    glBindBuffer(GL_ARRAY_BUFFER,ret.posVbo);
    glEnableVertexAttribArray(posAttrib);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts[0].x, GL_STATIC_DRAW);
    glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);


    glBindBuffer(GL_ARRAY_BUFFER,ret.uvVbo);
    glEnableVertexAttribArray(uvAttrib);
    glVertexAttribPointer( uvAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER,sizeof(coords), &coords[0].x, GL_STATIC_DRAW);

	glBindVertexArray(0);

	return ret;
}

void Skybox_Free(Skybox *skybox){
	glDeleteVertexArrays(1, &skybox->vao);
	glDeleteBuffers(1, &skybox->posVbo);
	glDeleteBuffers(1, &skybox->uvVbo);
}