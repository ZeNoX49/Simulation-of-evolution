#include "Tile.hpp"

#include <cmath>
#include <limits>
#include <algorithm>

#include "../gameParam.hpp"
#include "../utils/mathUtils.hpp"
#include "../utils/Noise.hpp"
#include "map.hpp"

Noise perlin = Noise();

float Tile::local_evap(float temp) {
    return MathUtils::clamp((temp - (-37.0f)) / (28.0f - (-37.0f)), 0.0f, 1.0f);
}

static inline int hexDistance(const Tile* a, const Tile* b) {
    int ax = a->hexCoord.x;
    int az = a->hexCoord.y;
    int ay = -ax - az;

    int bx = b->hexCoord.x;
    int bz = b->hexCoord.y;
    int by = -bx - bz;

    return (std::abs(ax - bx) + std::abs(ay - by) + std::abs(az - bz)) / 2;
}

float Tile::getDistToOcean(const std::vector<Tile*>& allWaterTiles) {
    // Si la tile elle-même est de l'eau
    if (biome.biomeType == BiomeType::Water)
        return 0.0f;

    // Sécurité si jamais la liste est vide
    if (allWaterTiles.empty())
        return static_cast<float>(std::numeric_limits<int>::max());

    int minDist = std::numeric_limits<int>::max();

    for (const Tile* water : allWaterTiles) {
        if (!water) continue;

        int d = hexDistance(this, water);
        minDist = std::min(minDist, d);
    }

    return static_cast<float>(minDist);
}

void Tile::computeClimate(const std::vector<Tile*>& allWaterTiles) {
    perlin.initPerm(gameParam::map_seed);

    float latitude_norm = abs((((float)this->hexCoord.y / (float)(gameParam::map_size-1)) - 0.5f) * 2);

    float T_equator = 28.0f;
    float lat_temp_gradient = 40.0f; // donne froid aux pôles
    float lapse_rate = 25.0f; // per altitude unit (si altitude normalisée 0..1)
    float noiseValue = perlin.fractalNoise(
        (gameParam::offsetX + this->hexCoord.x * gameParam::map_frequency) * -1,
        (gameParam::offsetY + this->hexCoord.y * gameParam::map_frequency) * -1,
        gameParam::map_octaves,
        gameParam::map_persistence,
        gameParam::map_lacunarity
    ); //petit bruit ±2°C
    float noise = noiseValue * 4 - 2;
    this->temperature = T_equator - (latitude_norm * lat_temp_gradient) - (this->height * lapse_rate) + noise;

    float map_diagonal = sqrt(2 * pow(gameParam::map_size, 2)); // gameParam::map_size*gameParam::map_size + gameParam::map_size*gameParam::map_size
    float decay_factor = 2.0f;
    float decay = map_diagonal * decay_factor;
    float moisture_ocean = exp(-getDistToOcean(allWaterTiles) / decay);
    float C = 1.2f;
    float D = 0.06f;
    float moisture_capacity = C * exp(D * this->temperature);
    float wind_inflow = 0.4f;
    float precip = std::min(
        moisture_ocean * wind_inflow + local_evap(this->temperature),
        moisture_capacity
    );
    this->precipitation = precip * 300;
}

void Tile::setBiomeAquatic() {
    this->biome = getBiome(BiomeType::Water);
}

void Tile::define_biome() {
    /* ---- Système de Whitaker simplifié ---- */
    // Tropical (≥ 20°C)
    if(this->temperature >= 20.0f) {
        if (this->precipitation >= 300.0f) {
            this->biome = getBiome(BiomeType::Tropical_Rainforest);
        }
        else if (this->precipitation >= 50.0f) {
            this->biome = getBiome(BiomeType::Tropical_Savanna);
        }
        else {
            this->biome = getBiome(BiomeType::Desert);
        }
    }
    
    // Temperate (5-20°C)
    else if(this->temperature >= 5.0f) {
        if (this->precipitation >= 200.0f) {
            this->biome = getBiome(BiomeType::Temperate_Rainforest);
        }
        else if (this->precipitation >= 100.0f) {
            this->biome = getBiome(BiomeType::Temperate_Deciduous_Forest);
        }
        else if (this->precipitation >= 25.0f) {
            this->biome = getBiome(BiomeType::Temperate_Grassland);
        }
        else {
            this->biome = getBiome(BiomeType::Desert);
        }
    }
    
    // Cold (-5 to 5°C)
    else if(this->temperature >= -5.0f) {
        if (this->precipitation >= 50.0f) {
            this->biome = getBiome(BiomeType::Taiga);
        }
        else {
            this->biome = getBiome(BiomeType::Desert);
        }
    }
    
    // Very Cold (< -5°C)
    else if (this->temperature >= -30.0f) {
        this->biome = getBiome(BiomeType::Tundra);
    }
    
    // Polar (< -30°C)
    else {
        this->biome = getBiome(BiomeType::Polar);
    }
}

Tile* Tile::getNeighbors(hexNeighbors neighbors_direction) {
    HexCoord coord;
    switch (neighbors_direction) {
        case hexNeighbors::Est:
            coord.x = this->hexCoord.x + 1.0f;
            coord.y = this->hexCoord.y;
            break;
        case hexNeighbors::Nord_Est:
            coord.x = this->hexCoord.x + 0.5f;
            coord.y = this->hexCoord.y - 1.0f;
            break;
        case hexNeighbors::Nord_Ouest:
            coord.x = this->hexCoord.x + 0.5f;
            coord.y = this->hexCoord.y - 1.0f;
            break;
        case hexNeighbors::Ouest:
            coord.x = this->hexCoord.x - 1.0f;
            coord.y = this->hexCoord.y;
            break;
        case hexNeighbors::Sud_Ouest:
            coord.x = this->hexCoord.x + 0.5f;
            coord.y = this->hexCoord.y + 1.0f;
            break;
        case hexNeighbors::Sud_Est:
            coord.x = this->hexCoord.x + 0.5f;
            coord.y = this->hexCoord.y + 1.0f;
            break;
    }

    auto it = map::hexmap.find(coord);
    if (it != map::hexmap.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<Tile*> Tile::getAllNeighbors() {
    std::vector<Tile*> neighbors;
    for (int i = 0; i < 6; ++i) {
        if(Tile* tile = getNeighbors(static_cast<hexNeighbors>(i))) {
            neighbors.push_back(tile);
        }
    }
    return neighbors;
}