#include "image_loader.h"
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <string.h>
#include <linux/limits.h>

typedef struct {
    Image img;
    char path[PATH_MAX];
} LoadedImage;

static LoadedImage *loadedImages;
static int numLoadedImages = 0;

void ImageLoader_BindImage(Image img){
    glBindTexture(GL_TEXTURE_2D, img.glTexture);
}

Image ImageLoader_CreateImage(const char *path, char loadMipMaps){

    int k;
    for(k = 0; k < numLoadedImages; k++)
        if(strcmp(loadedImages[k].path, path) == 0)
            return loadedImages[k].img;

    Image ret;

    FILE *fp = fopen(path,"rb");

    if( fp == NULL ){
        printf("Error loading PNG %s: No such file.\n", path);
        return ret;
    }

    unsigned char header[8];
    fread(header, 1, 8, fp);
    int ispng = !png_sig_cmp(header, 0, 8);

    if(!ispng){
        fclose(fp);
        printf("Not png %s\n", path);
        return ret;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr) {
        printf("Error loading %s\n",path );
        fclose(fp);
        return ret;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr){
        printf("Error loading %s\n",path );
        fclose(fp);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return ret;
    }

    if(setjmp(png_jmpbuf(png_ptr))){
        printf("Error loading %s\n",path );
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return ret;
    }

    glGenTextures(1, &ret.glTexture);

    png_set_sig_bytes(png_ptr, 8);
    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    int bit_depth, color_type;
    png_uint_32 twidth, theight;

    png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

    if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    if(bit_depth < 8)
        png_set_packing(png_ptr);

    if(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY)
        png_set_add_alpha(png_ptr, 255, PNG_FILLER_AFTER);

    png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

    ret.w = twidth;
    ret.h = theight;

    png_read_update_info(png_ptr, info_ptr);

    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    png_byte *image_data = (png_byte *)malloc(sizeof(png_byte) * rowbytes * ret.h);
    if(!image_data){
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return ret;
    }

    png_bytep *row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * ret.h);
    if(!row_pointers){
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        free(image_data);
        return ret;
    }

    int i;
    for(i = 0; i < ret.h; ++i)
        row_pointers[ret.h - 1 - i] = image_data + i * rowbytes;

    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, NULL);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    free(row_pointers);
    fclose(fp);

    glBindTexture(GL_TEXTURE_2D, ret.glTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ret.w, ret.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);


    if(loadMipMaps){
        glGenerateMipmap(GL_TEXTURE_2D);
        ImageLoader_SetImageParameters(ret, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT);
    }

    free(image_data);

    glBindTexture(GL_TEXTURE_2D, 0);

    loadedImages = (LoadedImage *)realloc(loadedImages, sizeof(LoadedImage) * ++numLoadedImages);
    loadedImages[numLoadedImages-1].img = ret;

    strncpy(loadedImages[numLoadedImages-1].path, path, sizeof(loadedImages[numLoadedImages-1].path));

    return ret;
}

void ImageLoader_SetImageParameters(Image img, int minFilter, int magFilter, int glTextureWrapS, int glTextureWrapT){
    glBindTexture(GL_TEXTURE_2D, img.glTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,glTextureWrapS);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,glTextureWrapT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ImageLoader_DeleteImage(Image *img){

    int k;
    for(k = 0; k < numLoadedImages; k++){
        if(loadedImages[k].img.glTexture == img->glTexture){

            int f;
            for(f = k; f < numLoadedImages-1; f++)
                loadedImages[f] = loadedImages[f+1];

            loadedImages = (LoadedImage *)realloc(loadedImages, sizeof(LoadedImage) * --numLoadedImages);
        }
    }

    glDeleteTextures(1, &img->glTexture);
    img->glTexture = 0;
}

void ImageLoader_Free(){

    int k;
    for(k = 0; k < numLoadedImages; k++)
        glDeleteTextures(1, &loadedImages[k].img.glTexture);

    if(loadedImages) free(loadedImages);
    loadedImages = NULL;
    numLoadedImages = 0;
}