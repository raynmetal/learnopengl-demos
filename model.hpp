#ifndef ZOMODEL_H
#define ZOMODEL_H

#include <string>
#include <vector>
#include <map>

#include <assimp/scene.h>

#include "shader.hpp"
#include "mesh.hpp"

class Model {
public:
    Model(const std::string& path, const Shader& shader);
    void Draw(const Shader& shader) const;

private:
    // model data
    std::vector<Mesh> meshes;
    std::map<std::string, bool> isTextureLoaded;
    std::map<std::string, Texture> loadedTexture;
    std::string directory;
    std::string modelPath;

    void loadModel(const std::string& path, const Shader& shader);
    void processNode(aiNode* node, const aiScene* scene, const Shader& shader);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene, const Shader& shader);
    std::vector<Texture*> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
};

#endif
