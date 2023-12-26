#ifndef ZOMESH_H
#define ZOMESH_H

#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "shader.hpp"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

class Mesh {
    GLuint vao, vbo, ebo;
    void setupMesh(const Shader& shader);

public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture*> textures;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, const std::vector<Texture*>& textures, const Shader& shader);
    void Draw (const Shader& shader) const;
};

#endif
