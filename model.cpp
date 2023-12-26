#include <iostream>
#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"

#include "model.hpp"

Model::Model(const std::string& path, const Shader& shader): isTextureLoaded {}, modelPath {path} {
    loadModel(path, shader);
}

void Model::Draw(const Shader& shader) const {
    for(std::size_t i {0}; i < meshes.size(); ++i){
        meshes[i].Draw(shader);
    }
}

void Model::loadModel(const std::string& path, const Shader& shader) {
    //create an instance of an assimp model importer
    Assimp::Importer importer;

    // Get a pointer to the model's scene object
    const aiScene* scene {
        importer.ReadFile(
            path,  // model file path (.fbx, .wav, etc.,)
            // Post processing options:
            (
                // Convert portions of the model not 
                // defined by triangles into triangles
                aiProcess_Triangulate 

                // Flip texture sampling coordinates 
                // upside down. OpenGL expects (0, 0) to correspond
                // to an image's bottom left, but images are 
                // usually read from top left -> bottom right
                // | aiProcess_FlipUVs 
            )
        )
    };

    // Check for model loading failure; report errors
    // if any
    if(
        !scene 
        || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
        || !(scene->mRootNode)
    ){
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    //get path to the directory containing the model
    this->directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene, shader);
}

void Model::processNode(aiNode* node, const aiScene* scene, const Shader& shader) {
    //Process all the node's meshes, if any
    for(std::size_t i {0}; i < node->mNumMeshes; ++i) {
        aiMesh *mesh { scene->mMeshes[node->mMeshes[i]] };
        meshes.push_back(processMesh(mesh, scene, shader));
    }

    //Recursively process this node's children
    for(std::size_t i{0}; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene, shader);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene, const Shader& shader) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture*> textures;

    //load vertices
    for(std::size_t i{0}; i < mesh->mNumVertices; ++i) {
        Vertex vertex {
            // position
            {
                mesh->mVertices[i].x,
                mesh->mVertices[i].y, 
                mesh->mVertices[i].z
            },
            // normals
            {
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            },
            // texture coordinates
            {
               mesh->mTextureCoords[0][i].x,
               mesh->mTextureCoords[0][i].y
            }
        };
        vertices.push_back(vertex);
    }

    // load indices
    // Each face on a mesh indexes 3 vertices, as a result
    // of us using aiProcessTriangulate
    for(std::size_t i {0}; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for(std::size_t j{0}; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // load textures
    if(mesh->mMaterialIndex >= 0) {
        aiMaterial *material { scene->mMaterials[mesh->mMaterialIndex] };

        //Get a list of all diffuse maps associated with this material
        std::vector<Texture*> diffuseMaps {
            loadMaterialTextures(
                material, aiTextureType_DIFFUSE, "texture_diffuse"
            )
        };
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        //Get a list of all specular maps associated with this material
        std::vector<Texture*> specularMaps { 
            loadMaterialTextures(
                material, aiTextureType_SPECULAR, "texture_specular"
            )
        };
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return Mesh {
        vertices, indices, textures, shader
    };
}

std::vector<Texture*> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName) {
    std::vector<Texture*> textures {};

    for(std::size_t i{0}; i < mat->GetTextureCount(type); ++i) {

        //Retrieve the name of the textures
        aiString textureNameAi;
        mat->GetTexture(type, i, &textureNameAi);
        std::string textureName {textureNameAi.C_Str()};

        // Ensure that reused textures are loaded just once from file
        if(!isTextureLoaded[textureName]){
            Texture texture {
                //texture image file name, assuming it's located in the
                //same directory as the model that uses it
                directory + std::string("/") + textureName,
                typeName
            };
            loadedTexture[textureName] = std::move(texture);
            isTextureLoaded[textureName] = true;
        }

        textures.push_back(&loadedTexture[textureName]);
    }

    return textures;
}
