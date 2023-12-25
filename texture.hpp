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
    void bindTexture(bool bind = true) const;

    // Getter functions
    GLuint getTextureID() const;
    std::string getType() const;

private:
    GLuint mID;
    std::string type;
};

#endif
