#ifndef ZOSHADER_H
#define ZOSHADER_H

#include <string>
#include <map>

#include <GL/glew.h>


class Shader {
public:
    // Constructor, reads and builds shader
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    //use/activate this shader
    void use();
    bool getBuildSuccess();

    //utility attrib array functions
    GLint attribLocation(const std::string& name) const;
    void enableAttribArray(const std::string& name) const;
    void disableAttribArray(const std::string& name) const;
    void setAttribPointerF(const std::string& name, int nComponents, int stride, int offset) const;

    //utility uniform functions
    GLint uniformLocation(const std::string& name) const;
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;

    GLuint getProgramID() { return mID; }

private:
    // program ID
    GLuint mID;
    bool mBuildState;
};

#endif
