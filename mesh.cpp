#include <string>
#include <vector>
#include <GL/glew.h>

#include "shader.hpp"
#include "texture.hpp"
#include "mesh.hpp"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, const std::vector<Texture>& textures, const Shader& shader):
    vertices{vertices}, indices{indices}, textures{textures}
{
    setupMesh(shader);
}

void Mesh::setupMesh(const Shader& shader) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
        // load vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        // load element buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

        // Pointers to various interleaved vertex properties
        shader.enableAttribArray("position");
        shader.setAttribPointerF("position", 3, sizeof(Vertex)/sizeof(float), offsetof(Vertex, position)/sizeof(float));
        shader.enableAttribArray("normal");
        shader.setAttribPointerF("normal", 3, sizeof(Vertex)/sizeof(float), offsetof(Vertex, normal)/sizeof(float));
        shader.enableAttribArray("textureCoord");
        shader.setAttribPointerF("textureCoord", 2, sizeof(Vertex)/sizeof(float), offsetof(Vertex, texCoords)/sizeof(float));
    glBindVertexArray(0);
}

void Mesh::Draw (Shader& shader) const {
    unsigned int diffuseN {1};
    unsigned int specularN {1};

    // bind textures to texture units in GPU
    for(unsigned int i{0}; i < textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string name { textures[i].getType() };
        std::string number { std::to_string(name == "texture_diffuse"? diffuseN++: specularN++) };
        shader.setInt(("material." + name + number).c_str(), i);
        textures[i].bindTexture();
    }
    glActiveTexture(GL_TEXTURE0);

    // draw mesh
    glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
