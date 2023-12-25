#include <string>
#include <iostream>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "texture.hpp"
#include "utility.hpp"

void flip_surface(SDL_Surface* surface);

Texture::Texture(const std::string& filepath, const std::string& type): type{type} {
    bool success { loadTextureFromFile(filepath.c_str()) };
    if(!success) {
        std::cout << "Could not load texture from " << filepath << '!' << std::endl;
    } else std::cout << "Texture at " << filepath << " loaded successfully!" << std::endl;
}

Texture::Texture(GLuint textureID, const std::string& type):
    mID{textureID}, type{type}
{}

Texture::~Texture() {
    freeTexture();
}

void Texture::freeTexture() { 
    if(!mID) return;
    glDeleteTextures(1, &mID);
    mID = 0;
}

bool Texture::loadTextureFromFile(const char* filename) {
    freeTexture();

    // Load image from file into an SDL surface
    SDL_Surface* texture_image { IMG_Load(filename) };
    if(!texture_image) {
        std::cout << "Could not load texture!\n" 
            << IMG_GetError() << std::endl;
        return false;
    }

    //Convert image from RGBA -> RGB
    SDL_Surface* pretexture = SDL_ConvertSurfaceFormat(texture_image, SDL_PIXELFORMAT_RGB24, 0 );
    SDL_FreeSurface(texture_image);
    texture_image = nullptr;
    if(!pretexture) {
        std::cout << "Something went wrong: " << SDL_GetError() << std::endl;
        return false;
    }

    // Flip texture vertically before loading them into OpenGL
    // (OpenGL expects 0 as bottom, 1 as top, and SDL expects
    // the opposite)
    flip_surface(pretexture);

    // Move surface pixels to graphics card
    GLuint texture;
    glGenTextures(1, &texture);
    if(!texture) {
        SDL_FreeSurface(pretexture);
        return false;
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
        pretexture->w, pretexture->h,
        0, GL_RGB, GL_UNSIGNED_BYTE, 
        reinterpret_cast<void*>(pretexture->pixels)
    );
    SDL_FreeSurface(pretexture);
    pretexture = nullptr;

    // Verify that no errors occurred while copying
    // texture to video memory
    if(glGetError() != GL_NO_ERROR) {
        std::cout << "Could not convert image to OpenGL texture!\n"
            << glewGetErrorString(glGetError()) << std::endl;
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &texture);
        return false;
    }

    // Generate mipmaps, set some texture params
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        // Interpolate between close mipmaps, interpolating linearly
        // between nearby pixels within each texture
        GL_LINEAR_MIPMAP_LINEAR 
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    mID = texture;

    //Unbind texture
    bindTexture(false);

    //return success
    return true;
}

void Texture::bindTexture(bool bind) const {
    if(!bind || !mID){
        glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }
    glBindTexture(GL_TEXTURE_2D, mID);
}

GLuint Texture::getTextureID() const { return mID; }
std::string Texture::getType() const { return type; }

void flip_surface(SDL_Surface* surface) {
    if(!surface) return;

    //Lock surface for modification
    SDL_LockSurface(surface);

    int rowLen {surface->pitch}; //rowlength, in bytes
    char* temp { new char[rowLen] };
    char* pixels {reinterpret_cast<char*>(surface->pixels)};

    for(int i{0}; i < surface->h/2; ++i) {
        // pointers to the rows to be swapped
        char* row_top { pixels + i*rowLen };
        char* row_bottom {pixels + (surface->h - 1 - i ) * rowLen};

        //swap rows
        memcpy(temp, row_top, rowLen);
        memcpy(row_top, row_bottom, rowLen);
        memcpy(row_bottom, temp, rowLen);
    }
    delete[] temp;

    //Done modifying this surface, so unlock
    SDL_UnlockSurface(surface);
}
