#pragma once

#include "Biome.hpp"
#include "HexCoord.hpp"

class Tile {
public :
    HexCoord hexCoord;
    float height;
    float temperature;
    float precipitation;
    Biome biome = getBiome(BiomeType::None);

    void computeClimate(const std::vector<Tile*>& allWaterTiles);

    void setBiomeAquatic();
    void define_biome();

    Tile* getNeighbors(hexNeighbors neighbors);
    std::vector<Tile*> getAllNeighbors();

private :
    float local_evap(float temp);
    float getDistToOcean(const std::vector<Tile*>& allWaterTiles);
};