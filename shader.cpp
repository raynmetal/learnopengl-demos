#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"


Shader::Shader(const char* vertexPath, const char* fragmentPath) :mBuildState{false} {
    //Vertex and fragment shader file pointers and sources
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    //Enable exceptions on filepointers
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    bool readSuccess {true};

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);

        // Read file buffer's contents into string stream
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        //Close file handles
        vShaderFile.close();
        fShaderFile.close();

        //convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

    } catch(std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
        readSuccess = false;
    }
    if(!readSuccess) return;

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    GLuint vertexShader, fragmentShader;
    GLint success;
    char infoLog[512];

    //create vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(success != GL_TRUE) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        glDeleteShader(vertexShader);
        return;
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(success != GL_TRUE) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" 
            << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return;
    }

    //Create a shader program
    mID = glCreateProgram();
    glAttachShader(mID, vertexShader);
    glAttachShader(mID, fragmentShader);
    glLinkProgram(mID);
    glGetProgramiv(mID, GL_LINK_STATUS, &success);
    if(success != GL_TRUE) {
        glGetProgramInfoLog(mID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
            << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(mID);
        mID = 0;
        return;
    }

    //Clean up (now unnecessary) shader objects
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Store build success
    mBuildState = true;
}
Shader::~Shader() {
    glDeleteProgram(mID);
}

void Shader::use() {
    glUseProgram(mID);
}
bool Shader::getBuildSuccess() { return mBuildState; }

GLint Shader::attribLocation(const std::string& name) const {
    return glGetAttribLocation(mID, name.c_str());
}
GLint Shader::uniformLocation(const std::string& name) const {
    return glGetUniformLocation(mID, name.c_str());
}
void Shader::enableAttribArray(const std::string& name) const {
    glEnableVertexAttribArray(attribLocation(name));
}
void Shader::disableAttribArray(const std::string& name) const {
    glDisableVertexAttribArray(attribLocation(name));
}
void Shader::setAttribPointerF(const std::string& name, int nComponents, int stride, int offset) const {
    glVertexAttribPointer(
        attribLocation(name),
        nComponents, // number of components per elements
        GL_FLOAT, // data format
        GL_FALSE, // whether to normalize the data or not
        stride * sizeof(float), // no. of bytes between elements
        reinterpret_cast<void*>(offset*sizeof(float)) // offset to the first element, in bytes
    );
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(
        uniformLocation(name),
        static_cast<GLint>(value)
    );
}
void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(
        uniformLocation(name),
        static_cast<GLint>(value)
    );
}
void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(
        uniformLocation(name),
        static_cast<GLfloat>(value)
    );
}
void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(
        uniformLocation(name),
        1,
        glm::value_ptr(value)
    );
}
void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(
        uniformLocation(name),
        1,
        GL_FALSE,
        glm::value_ptr(value)
    );
}
