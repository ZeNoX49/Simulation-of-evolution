#pragma once

#include "rendering/Camera.hpp"

namespace gameParam
{
    /* carte */
    inline int map_size = 35;
    inline int map_seed = 54;
    inline int map_octaves = 3;
    inline float map_persistence = 1.4f;
    inline float map_lacunarity = 2.3f;
    inline float map_frequency = 0.01f;
    inline int offsetX = 0;
    inline int offsetY = 0;

    /* Aquatique */
    inline float water_threshold = 0.250f;
    inline float flow_threshold = 0.150f;
    inline int flow_mult = 1000;
    inline int nbVN = 3;

    /* --- */
    inline float min_temp = -40.0f;
    inline float max_temp = 30.0f;
    inline float max_precipitation = 325.0f;

    /* Debugging */
    inline int tile_color = 0;
    inline bool showWaterLevel = false;
    inline bool showMaxHeight = false;
}

namespace gameUtils
{
    inline Camera cam = Camera{glm::vec3{0, 0, 0}, 25.0f, 0.5f, 0.0f};
}
