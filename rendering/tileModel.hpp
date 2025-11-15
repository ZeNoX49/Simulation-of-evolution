#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 color;
};

struct ObjData {
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    std::vector<unsigned int> indices;
    std::vector<Vertex> vertices;
};

ObjData createTileModel(float x, float y, float h, float r, glm::vec3 color);