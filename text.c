#include <GL/glew.h>
#include "math.h"
#include "shaders.h"
#include <stdio.h>
#include "text.h"

static unsigned int coordVbo, colorVbo, posVbo, vao;

static float rgbToGL(float rgb){ return rgb / 255.0; }
static FT_Library  ftLibrary;

int FontFace_LoadFont(FontFace *font, const char *path){

    font->fontFace = NULL;
    font->fontTexture = 0;

    if(FT_New_Face(ftLibrary, path, 0, &font->fontFace)){
        printf("FontFace_LoadFont: Could not load font.\n");
        return 0;
    }

    return 1;
}

int FontFace_SetSize(FontFace *font, int size){

    font->fontSize = size;

    FT_Set_Pixel_Sizes(font->fontFace, 0, size);
    // FT_Set_Char_Size(font->fontFace, size << 6, size << 6, 96, 96);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &font->fontTexture);
    glBindTexture(GL_TEXTURE_2D, font->fontTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

    int w = 0, h = 0, i = 0, x = 0;

    FT_GlyphSlot g = font->fontFace->glyph;

    for(i = 32; i < 128; i++){
        if(FT_Load_Char(font->fontFace, i, FT_LOAD_RENDER)){
            printf("FontFace_SetSize: Error \n");
            continue;
        }
        w += g->bitmap.width;
        h  = h > g->bitmap.rows ? h : g->bitmap.rows;
    }

    font->fontHeight = h;

    font->atlasWidth  = w;
    font->atlasHeight = h;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,w,h,0,GL_RED,GL_UNSIGNED_BYTE,0);

    for(i = 32; i < 128; i++){

        if(FT_Load_Char(font->fontFace, i, FT_LOAD_RENDER)){
            printf("FontFace_SetSize: Error \n");
            continue;
        }

        // unsigned char data[g->bitmap.width * g->bitmap.rows];

        // int y, b;
        // for(y = 0; y < g->bitmap.rows; y++){
        //     for(b = 0; b < g->bitmap.pitch; b++){

        //         int val = g->bitmap.buffer[(y * g->bitmap.pitch) + b];

        //         int len = g->bitmap.width - (b * 8);

        //         int rowStart = (y * g->bitmap.width) + (b * 8);

        //         int k;

        //         for(k = 0; k < (len > 8 ? 8: len); k++){

        //             int bit = val & (1 << (7 - k));

        //             data[(rowStart + k)] = bit > 0 ? 255 : 0;
        //         }
        //     }
        // }

        // glTexSubImage2D(GL_TEXTURE_2D,0,x,0,g->bitmap.width,g->bitmap.rows,GL_RED,GL_UNSIGNED_BYTE,data);

        glTexSubImage2D(GL_TEXTURE_2D,0,x,0,g->bitmap.width,g->bitmap.rows,GL_RED,GL_UNSIGNED_BYTE,g->bitmap.buffer);

        font->fontCharacters[i].ax = g->advance.x >> 6;
        font->fontCharacters[i].ay = g->advance.y >> 6;
        font->fontCharacters[i].bw = g->bitmap.width;
        font->fontCharacters[i].bh = g->bitmap.rows;
        font->fontCharacters[i].bl = g->bitmap_left;
        font->fontCharacters[i].bt = g->bitmap_top;
        font->fontCharacters[i].tx = ((float)x / (float)w);
        x += g->bitmap.width;
    }
    return 1;
}

int Text_DrawText(Text *text){

    Shaders_UseProgram(TEXT_2D_SHADER);

    glBindVertexArray(vao);

    Shaders_UpdateProjectionMatrix();
    Shaders_UpdateViewMatrix();
    Shaders_UpdateModelMatrix();

    Shaders_SetUniformColor(text->color);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, text->fontFace->fontTexture);

    if(text->numVerts > 0){

        glBindBuffer(GL_ARRAY_BUFFER, posVbo);
        glBufferData(GL_ARRAY_BUFFER, text->numVerts*sizeof(Vec3), &text->verts[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, coordVbo);
        glBufferData(GL_ARRAY_BUFFER, text->numVerts*sizeof(Vec2), &text->texCoords[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
        glBufferData(GL_ARRAY_BUFFER, text->numVerts*sizeof(Vec4), &text->colors[0], GL_STATIC_DRAW);

        glDrawArrays(GL_TRIANGLES, 0, text->numVerts);
    }

    return 1;
}

Vec2 FontFace_GetCursorPos(FontFace *font, const char *text, int len, float sx, float sy){

    if(!text) return (Vec2){0,0};

    Vec2 cursorPos = (Vec2){0,0};

    int k;
    for(k = 0; k < len; k++){

        char p = text[k];

        if(text[k] == '\n'){
            cursorPos.y += font->fontSize * sy;
            cursorPos.x = 0;
            continue;
        }
        
        if(p == '\t' ){
            cursorPos.x += font->fontCharacters[(int)' '].ax * sx * TAB_SPACING;
            continue;
        }

        cursorPos.x += sx * font->fontCharacters[(int)text[k]].ax;
    }

    return cursorPos;
}


Vec2 FontFace_GetTextSize(FontFace *font, const char *text, int len, float sx, float sy){

    if(!text) return (Vec2){0,font->fontSize * sy};

    Vec2 cursorPos = (Vec2){0,font->fontSize * sy};

    float lineWidth = 0;

    int k;
    for(k = 0; k < len; k++){

        char p = text[k];

        if(text[k] == '\n'){
            cursorPos.y += font->fontSize * sy;
            if(lineWidth > cursorPos.x) cursorPos.x = lineWidth;
            lineWidth = 0;
            continue;
        }
        
        if(p == '\t' ){
            lineWidth += font->fontCharacters[(int)' '].ax * sx * TAB_SPACING;
            continue;
        }

        lineWidth += sx * font->fontCharacters[(int)text[k]].ax;
    }

    if(lineWidth > cursorPos.x) cursorPos.x = lineWidth;

    return cursorPos;
}

void Text_AddText(Text *textData, const char *text, float x, float y, float z, float sx, float sy){

    float startX = x;

    Vec4 currColor = (Vec4){1,1,1,1};

    y += textData->fontFace->fontSize*sy;

    int m;
    for(m = 0; m < (int)strlen(text); m++){

        char p = text[m];

        if(p == '\n'){
            y += textData->fontFace->fontSize*sy;
            x = startX;
            continue;
        }

        if(p == '\t' ){
            x += textData->fontFace->fontCharacters[(int)' '].ax * sx * TAB_SPACING;
            continue;
        }

        else if(p == (char)COLOR_CHAR1){ currColor = (Vec4){rgbToGL(248),rgbToGL(248),rgbToGL(242),1}; continue; }
        else if(p == (char)COLOR_CHAR2){ currColor = (Vec4){rgbToGL(255),rgbToGL(102),rgbToGL(0),  1}; continue; }
        else if(p == (char)COLOR_CHAR3){ currColor = (Vec4){rgbToGL(102),rgbToGL(255),rgbToGL(0),1}; continue; }
        else if(p == (char)COLOR_CHAR4){ currColor = (Vec4){rgbToGL(204),rgbToGL(255),rgbToGL(51),1}; continue; }
        else if(p == (char)COLOR_CHAR5){ currColor = (Vec4){rgbToGL(125),rgbToGL(193),rgbToGL(207),1}; continue; }
        else if(p == (char)COLOR_CHAR6){ currColor = (Vec4){rgbToGL(153),rgbToGL(51),rgbToGL(204),1}; continue; }
        else if(p == (char)COLOR_CHAR7){ currColor = (Vec4){rgbToGL(170),rgbToGL(255),rgbToGL(255),1}; continue; }
        else if(p == (char)COLOR_CHAR8){ currColor = (Vec4){rgbToGL(0),rgbToGL(0),rgbToGL(0),1}; continue; }

        FontCharacter charac = textData->fontFace->fontCharacters[(int)p];

        float x2 =  x + charac.bl * sx;
        float y2 = -y + charac.bt * sy;
        float w = charac.bw * sx;
        float h = charac.bh * sy;

        x += charac.ax * sx;
        y += charac.ay * sy;

        if(!w || !h) continue;

        textData->numVerts += 6;

        textData->verts = (Vec3 *)realloc(textData->verts, sizeof(Vec3) * textData->numVerts);
        textData->texCoords = (Vec2 *)realloc(textData->texCoords, sizeof(Vec2) * textData->numVerts);
        textData->colors = (Vec4 *)realloc(textData->colors, sizeof(Vec4) * textData->numVerts);

        int f;
        for(f = 0; f < 6; f++)
            textData->colors[textData->colorsIndex++] = currColor;

        textData->verts[textData->vertIndex++] = (Vec3){ x2,     -y2    , z};
        textData->verts[textData->vertIndex++] = (Vec3){ x2,     -y2 + h, z};
        textData->verts[textData->vertIndex++] = (Vec3){ x2 + w, -y2    , z};
        textData->verts[textData->vertIndex++] = (Vec3){ x2 + w, -y2    , z};
        textData->verts[textData->vertIndex++] = (Vec3){ x2,     -y2 + h, z};
        textData->verts[textData->vertIndex++] = (Vec3){ x2 + w, -y2 + h, z};

        int ah = textData->fontFace->atlasHeight;
        int aw = textData->fontFace->atlasWidth;

        textData->texCoords[textData->coordIndex++] = (Vec2){ charac.tx, 0};
        textData->texCoords[textData->coordIndex++] = (Vec2){ charac.tx, (charac.bh / ah) };
        textData->texCoords[textData->coordIndex++] = (Vec2){ charac.tx + (charac.bw / aw),  0 };
        textData->texCoords[textData->coordIndex++] = (Vec2){ charac.tx + (charac.bw / aw),  0 };
        textData->texCoords[textData->coordIndex++] = (Vec2){ charac.tx,  (charac.bh / ah) };
        textData->texCoords[textData->coordIndex++] = (Vec2){ charac.tx + (charac.bw / aw),(charac.bh / ah)};
    }
}

void Text_Clear(Text *text){
    if(text->verts) free(text->verts);
    if(text->texCoords)free(text->texCoords);
    if(text->colors)free(text->colors);
    text->colors = NULL;
    text->texCoords = NULL;
    text->verts = NULL;
    text->numVerts = 0;
    text->vertIndex = 0;
    text->colorsIndex = 0;
    text->coordIndex = 0;
}

Text Text_Create(FontFace *font){
    Text ret;
    memset(&ret, 0, sizeof(Text));
    ret.color = (Vec4){1,1,1,1};
    ret.fontFace = font;
    return ret;
}

void FontFace_Delete(FontFace *font){
    if(font->fontTexture != 0) glDeleteTextures(1,&font->fontTexture);
    if(font->fontFace) FT_Done_Face(font->fontFace);
}

void Text_Close(){
    if(ftLibrary) FT_Done_FreeType(ftLibrary);
    ftLibrary = NULL;

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &posVbo);
    glDeleteBuffers(1, &coordVbo);
    glDeleteBuffers(1, &colorVbo);
}

void Text_Init(){
    if(FT_Init_FreeType(&ftLibrary)) printf("Could not Init Freetype.\n");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLint posLoc = glGetAttribLocation(Shaders_GetProgram(TEXT_2D_SHADER),SHADERS_POSITION_ATTRIB);
    GLint uvLoc = glGetAttribLocation(Shaders_GetProgram(TEXT_2D_SHADER), SHADERS_COORD_ATTRIB);
    GLint colorLoc = glGetAttribLocation(Shaders_GetProgram(TEXT_2D_SHADER), SHADERS_COLOR_ATTRIB);

    if(posLoc != -1){
        glGenBuffers(1, &posVbo);
        glBindBuffer(GL_ARRAY_BUFFER,posVbo);
        glEnableVertexAttribArray(posLoc);
        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    if(colorLoc != -1){
        glGenBuffers(1, &colorVbo);
        glBindBuffer(GL_ARRAY_BUFFER,colorVbo);
        glEnableVertexAttribArray(colorLoc);
        glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
    }
    if(uvLoc != -1){
        glGenBuffers(1, &coordVbo);
        glBindBuffer(GL_ARRAY_BUFFER,coordVbo);
        glEnableVertexAttribArray(uvLoc);
        glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glBindVertexArray(0);
}
