/*
 * image.cpp
 *
 *  Created on: Sep 16, 2011
 *      Author: claudio
 *
 *  Image support
 */

#include <GL/glut.h>
#include <FreeImage.h>
#include <cassert>
#include <iostream>
#include "cprocessing.hpp"
#include "pimage.hpp"

using namespace cprocessing;

namespace cprocessing {


    PImage::PImage() {
        this->width = 0;
        this->height = 0;
        this->type = ARGB;
        this->texturebuffer = 0;
    }

    PImage::PImage (int width, int height, unsigned type) {
        assert (width>=0 && height>=0 && 
                width < 1e6 && height < 1e6 && 
               (type == ARGB || type == RGB || type == ALPHA));

        this->width = width;
        this->height = height;
        this->type = type;
        this->texturebuffer = 0;
    }
    
    /// Destructor
    //TODO
    PImage::~PImage () {
        glDeleteTextures(1, &textureID);
        if(texturebuffer) delete texturebuffer;
    }

    
    PImage& PImage::get() {
        return (*this);
    }

    color PImage::get(int x, int y) {
        return pixels.buffertocolor(((h-y)*w)+x);
    }

    void PImage::set(int x, int y, const color& c) {
        pixels.colortobuffer(((h-y)*w)+x, c);
    }


    /// Draws the image at position (x, y) of the screen
    void PImage::put (int x, int y, int w, int h) {
        glEnable(GL_TEXTURE_2D);

        GLdouble vertices[] = { 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0 };
        GLdouble texture[]  = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

        // activate and specify pointer to vertex array
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glVertexPointer(2, GL_DOUBLE, 0, vertices);
        glTexCoordPointer(2, GL_DOUBLE, 0, texture);


        glPushMatrix();
        glTranslatef(x, y, 0.0);
        glScalef(w, h, 1.0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor4ubv (styles[styles.size()-1].fillColor.rgba);
        glDrawArrays(GL_QUADS,0,4);
        glPopMatrix();

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_TEXTURE_2D);
    }

    void PImage::loadImage(const char * src) {
        FREE_IMAGE_FORMAT format = FreeImage_GetFileType(src,0);
        FIBITMAP * imagen = FreeImage_Load(format, src);
        FIBITMAP * temp = imagen;
        imagen = FreeImage_ConvertTo32Bits(imagen);
        FreeImage_Unload(temp);
     
        w = FreeImage_GetWidth(imagen);
        h = FreeImage_GetHeight(imagen);
     
        texturebuffer = new GLubyte[4*w*h];
        char * tempbuffer = (char*)FreeImage_GetBits(imagen);

        //FreeImage loads in BGR format, so you need to swap some bytes(Or use GL_BGR).
        for(int j= 0; j<w*h; j++){
            texturebuffer[j*4+0]= tempbuffer[j*4+2];
            texturebuffer[j*4+1]= tempbuffer[j*4+1];
            texturebuffer[j*4+2]= tempbuffer[j*4+0];
            texturebuffer[j*4+3]= tempbuffer[j*4+3];
        }

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGBA, GL_UNSIGNED_BYTE, texturebuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)texturebuffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        this->pixels.setBuffer(texturebuffer);
    }

    void PImage::save(const char * out) {
        FIBITMAP *bitmap = FreeImage_Allocate(w, h, 24);
        RGBQUAD c;
        if (!bitmap) exit(1);

        for(int y=0; y<h; y++) {
            for(int x=0; x<w; x++) {
                c.rgbRed    = texturebuffer[(((y*w)+x)*4)+0];
                c.rgbGreen  = texturebuffer[(((y*w)+x)*4)+1];
                c.rgbBlue   = texturebuffer[(((y*w)+x)*4)+2];
                FreeImage_SetPixelColor(bitmap, x, y, &c);
            }
        }

        FreeImage_Save(FIF_PNG, bitmap, out, 0);
        FreeImage_Unload(bitmap);
    }
}

