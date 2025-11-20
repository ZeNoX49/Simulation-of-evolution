#pragma once

#include "rendering/Camera.hpp"
#include "environment/TerrainGenerator.hpp"

namespace gameParam
{
    /* carte */
    inline int map_size = 35;
    inline int map_seed = 54;
    inline int map_octaves = 3;
    inline float map_persistence = 1.4f;
    inline float map_lacunarity = 2.3f;
    inline float map_frequency = 0.01f;

    /* Aquatique */
    inline float water_threshold = 0.250f;
    inline float erosion_iterations = 0.1f;
    inline float riverFlowThreshold = 0.1f;

    /* Debugging */
    inline int tile_color = 0;
    inline bool showWaterLevel = false;
    inline bool showMaxHeight = false;
}

namespace gameUtils
{
    inline Camera cam = Camera{glm::vec3{0, 0, 0}, 25.0f, 0.5f, 0.0f};
    inline TerrainGenerator terGen = TerrainGenerator(gameParam::map_seed);
}
