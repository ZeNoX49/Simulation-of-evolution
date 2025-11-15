#include "biome.hpp"

Biome getBiome(BiomeType biomeType) {
    if(biomeType == BiomeType::Water) {
        return Biome{biomeType, "Aquatique", glm::vec3(0, 0, 220)/255.0f};
    }
    if(biomeType == BiomeType::Tropical_Rainforest) {
        return Biome{biomeType, "Forêt tropicale humide", glm::vec3(14, 226, 121)/255.0f};
    }
    if(biomeType == BiomeType::Tropical_Savanna) {
        return Biome{biomeType, "Savane", glm::vec3(255, 218, 63)/255.0f};
    }
    if(biomeType == BiomeType::Temperate_Rainforest) {
        return Biome{biomeType, "Forêt tempérée humide", glm::vec3(21, 203, 106)/255.0f};
    }
    if(biomeType == BiomeType::Temperate_Deciduous_Forest) {
        return Biome{biomeType, "Forêt de feuillus", glm::vec3(18, 179, 92)/255.0f};
    }
    if(biomeType == BiomeType::Temperate_Grassland) {
        return Biome{biomeType, "Prairie", glm::vec3(234, 252, 52)/255.0f};
    }
    if(biomeType == BiomeType::Taiga) {
        return Biome{biomeType, "Taïga", glm::vec3(22, 154, 83)/255.0f};
    }
    if(biomeType == BiomeType::Desert) {
        return Biome{biomeType, "Désert", glm::vec3(255, 144, 165)/255.0f};
    }
    if(biomeType == BiomeType::Tundra) {
        return Biome{biomeType, "Toundra", glm::vec3(176, 217, 235)/255.0f};
    }
    return Biome{biomeType, "Polaire", glm::vec3(216, 227, 235)/255.0f};
}