#include <string>
#include <iostream>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "texture.hpp"
#include "utility.hpp"

Texture::Texture(const char* filename) {
    bool success { loadTextureFromFile(filename) };
    if(!success) {
        std::cout << "Could not load texture from " << filename << '!' << std::endl;
    }
}

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
    SDL_Surface* pretexture {
        SDL_CreateRGBSurface(
        0,
        nearestPowerOfTwo_32bit(texture_image->w),
        nearestPowerOfTwo_32bit(texture_image->h),
        24,
        0xff0000,
        0x00ff00,
        0x0000ff,
        0
    )};
    if(!pretexture) {
        std::cout << "Something went wrong: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(texture_image);
        return false;
    }
    SDL_SetSurfaceBlendMode(texture_image, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceBlendMode(pretexture, SDL_BLENDMODE_NONE);
    SDL_BlitSurface(texture_image, nullptr, pretexture, nullptr);
    SDL_FreeSurface(texture_image);
    texture_image = nullptr;

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

void Texture::bindTexture(bool bind) {
    if(!bind || !mID){
        glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }
    glBindTexture(GL_TEXTURE_2D, mID);
}

GLuint Texture::getTextureID() { return mID; }
