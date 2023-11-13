#ifndef ZOTEXTURE_H
#define ZOTEXTURE_H

#include <GL/glew.h>

class Texture {
public:
    Texture(const char* filename);
    ~Texture();

    // Basic alloc and dealloc functions
    bool loadTextureFromFile(const char* filename);
    void freeTexture();

    //Bind/unbind texture
    void bindTexture(bool bind = true);

    // Getter functions
    GLuint getTextureID();

private:
    GLuint mID;
};

#endif
