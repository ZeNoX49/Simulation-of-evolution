#pragma once

#include "rendering/Camera.hpp"

namespace game
{
    inline Camera cam = Camera{glm::vec3{0, 0, 0}, 25.0f, 0.5f, 0.0f};
    
    /* Paramètres généraux */
    inline int map_size = 35;

    /* Aquatique */
    inline float water_threshold = 0.250f;
    inline int water_seed = 54;
    inline int water_octaves = 3;
    inline float water_persistence = 1.4f;
    inline float water_lacunarity = 2.3f;
    inline float water_frequency = 0.01f;

    inline int climate_seed = 0;
    inline int tile_color = 0;
}
