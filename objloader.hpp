#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

bool loadOBJ(
    const std::string& path,
    std::vector<glm::vec3>& out_vertices,
    std::vector<glm::vec2>& out_uvs,
    std::vector<glm::vec3>& out_normals
);

#endif
