#pragma once

#include "Biome.hpp"
#include "HexCoord.hpp"

class Tile {
public :
    HexCoord hexCoord;
    float height;
    float temperature;
    float precipitation;
    bool isWater = false;
    float flow = 0.0f;
    Biome biome;

    void setBiomeAquatic();
    void define_biome(int nbAquaticNeighbors);

    Tile* getNeighbors(hexNeighbors neighbors);
    std::vector<Tile*> getAllNeighbors();

private :
    float compute_temperature(int nbAquaticNeighbors);
    float compute_precipitation(int nbAquaticNeighbors);
    float getLatitude();
};