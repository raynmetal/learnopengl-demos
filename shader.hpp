#ifndef ZOSHADER_H
#define ZOSHADER_H

#include <GL/glew.h>

#include <string>

class Shader {
public:
    // Constructor, reads and builds shader
    Shader(const char* vertexPath, const char* fragmentPath);

    //use/activate this shader
    void use();

    //utility uniform functions
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;

    GLuint getProgramID() { return mID; }
private:
    // program ID
    GLuint mID;
};

#endif
