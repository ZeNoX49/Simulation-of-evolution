#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>

#include "ObjData.hpp"

inline ObjData tileModelOriginal;

extern ObjData loadOBJ(const std::string& path);

inline void loadResources() {
    tileModelOriginal = loadOBJ("object/tile_high.obj");
}

ObjData createTileModel(float x, float y, float h, float r, glm::vec3 color);