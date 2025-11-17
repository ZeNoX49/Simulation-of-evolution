#pragma once

#include "rendering/Camera.hpp"

namespace gameCam
{
    inline Camera cam = Camera{glm::vec3{0, 0, 0}, 25.0f, 0.5f, 0.0f};
}

namespace gameParam
{
    /* Paramètres généraux */
    inline int map_size = 35;

    inline int tile_color = 0;
    inline bool showWaterLevel = false;
    inline bool showMaxHeight = false;

    /* Aquatique */
    inline float water_threshold = 0.250f;
    inline int water_seed = 54;
    inline int water_octaves = 3;
    inline float water_persistence = 1.4f;
    inline float water_lacunarity = 2.3f;
    inline float water_frequency = 0.01f;
}
