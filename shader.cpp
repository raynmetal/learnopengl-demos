#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <GL/glew.h>

#include "shader.hpp"


Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    //Vertex and fragment shader file pointers and sources
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    //Enable exceptions on filepointers
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

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
    }

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
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(success != GL_TRUE) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" 
            << infoLog << std::endl;
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
    }

    //Clean up unnecessary shader objects
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::use() {
    glUseProgram(mID);
}

void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(mID, name.c_str()), static_cast<GLint>(value));
}

void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(mID, name.c_str()), static_cast<GLint>(value));
}

void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(mID, name.c_str()), static_cast<GLfloat>(value));
}
