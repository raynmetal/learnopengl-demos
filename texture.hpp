#ifndef ZOTEXTURE_H
#define ZOTEXTURE_H

#include <string>

#include <GL/glew.h>

class Texture {
public:
    Texture(const std::string& filepath, const std::string& type);
    Texture(GLuint textureID, const std::string& type);
    ~Texture();

    // Basic alloc and dealloc functions
    bool loadTextureFromFile(const char* filename);
    void freeTexture();

    //Bind/unbind texture
    void bindTexture(bool bind = true);

    // Getter functions
    GLuint getTextureID();
    std::string getType();

private:
    GLuint mID;
    std::string type;
};

#endif
