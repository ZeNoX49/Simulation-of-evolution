#pragma once

#include <string>
#include <glm/glm.hpp>

enum class BiomeType : int {
    Water,
    Tropical_Rainforest,
    Tropical_Savanna,
    Temperate_Rainforest,
    Temperate_Deciduous_Forest,
    Temperate_Grassland,
    Taiga,
    Desert,
    Tundra,
    Polar,
};

struct Biome {
    BiomeType biomeType;
    std::string name;
    glm::vec3 color;
};

Biome getBiome(BiomeType biomeType);