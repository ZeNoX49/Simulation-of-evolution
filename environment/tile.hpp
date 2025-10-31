#pragma once

#include "biome.hpp"

struct Tile {
    float x;
    float y;
    float height;
    Biome biome;
};

Biome define_biome(float x, float y, float height);
float compute_temperature(float x, float y, float height);
float compute_precipitation(float x, float y, float height, float temperature);
float getLatitude(float y);