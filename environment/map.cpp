#include <cmath>
#include "map.hpp"
#include "../gameParam.hpp"
#include "../configuration.hpp"
#include "../utils/Noise.hpp"
#include "../environment/hexCoord.hpp"
#include "../utils/mathUtils.hpp"
#include "../object/tileModel.hpp"

// Perlin pour hauteur
Noise noise = Noise();

void createRivers(Tile& tile);
Tile* lowestNeighbor(Tile& tile);

void createHexmap() {
    map::hexmap.clear();
    map::hexmap_drawable.clear();
    
    // créer les hexagones
    const float h = conf::game_window_height_f / (1.0f + (gameParam::map_size - 1) * 0.75f);
    const float radius = h / 2.0f;
    const float w = std::sqrt(3.f) * radius;

    // Centrer la caméra
    float centerX = (gameParam::map_size * w) / 2.0f / conf::model_size_div;
    float centerZ = ((gameParam::map_size - 1) * (h * 0.75f)) / 2.0f / conf::model_size_div;
    gameUtils::cam.target = glm::vec3(centerX, 0.0f, centerZ);
    
    noise.initPerm(gameParam::map_seed);

    /* ----- génération des tiles ----- */
    for (int row = 0; row < gameParam::map_size; ++row) {
        int colsInRow = (row % 2 == 0) ? gameParam::map_size : gameParam::map_size - 1;

        for (int col = 0; col < colsInRow; ++col) {
            float gridX = col + ((row % 2) ? 0.5f : 0.0f);
            float gridY = row;
            HexCoord hc{gridX, gridY};
            
            float height = noise.fractalNoise(
                gameParam::offsetX + gridX * gameParam::map_frequency,
                gameParam::offsetY + gridY * gameParam::map_frequency,
                gameParam::map_octaves,
                gameParam::map_persistence,
                gameParam::map_lacunarity
            );

            // Créer la tuile
            Tile tile;
            tile.hexCoord = hc;
            tile.height = height;
            
            // Biome d'eau ?
            double value = 0.0f;
            for(int p = 1; p <= gameParam::nbVN; p++) {
                double v = noise.valueNoise(
                    (gameParam::offsetX + gridX * gameParam::map_frequency * gameParam::flow_mult) * p,
                    (gameParam::offsetY + gridY * gameParam::map_frequency * gameParam::flow_mult) * p
                );
                value += v * p;
            }

            if (tile.height < gameParam::water_threshold || value < gameParam::flow_threshold) {
                tile.setBiomeAquatic();
            }
            
            map::hexmap[hc] = tile;
        }
    }

    /* ----- rivières ----- */
    std::vector<Tile*> waterTiles;

    for (auto& [coord, tile] : map::hexmap) {
        if (tile.biome.biomeType == BiomeType::Water && tile.height > gameParam::water_threshold) {
            waterTiles.push_back(&tile);
        }
    }

    for (Tile* tile : waterTiles) {
        createRivers(*tile);
    }

    waterTiles.clear();
    for (auto& [coord, tile] : map::hexmap) {
        int nbAquaticNeighbors = 0;
        for (Tile* n : tile.getAllNeighbors()) {
            if (n->biome.biomeType == BiomeType::Water) nbAquaticNeighbors++;
        }
        if(nbAquaticNeighbors > 4) tile.setBiomeAquatic();

        if (tile.biome.biomeType == BiomeType::Water) {
            waterTiles.push_back(&tile);
        }
    }

    /* ----- hexagones ----- */
    for (int row = 0; row < gameParam::map_size; ++row) {
        int colsInRow = (row % 2 == 0) ? gameParam::map_size : gameParam::map_size - 1;

        for (int col = 0; col < colsInRow; ++col) {
            float x = col * w + ((row % 2) ? w / 2.0f : 0.0f);
            float y = row * (h * 0.75f);

            float gridX = col + ((row % 2) ? 0.5f : 0.0f);
            float gridY = row;

            float c;
            glm::vec3 color;
            ObjData hex;

            Tile& tile = map::hexmap[HexCoord{gridX, gridY}];
            // Déterminer le biome
            tile.computeClimate(waterTiles);
            if(tile.biome.biomeType != BiomeType::Water) {
                tile.define_biome();
            }

            switch (gameParam::tile_color) {
                case 0:   // Biome
                    color = tile.biome.color;
                    hex = createTileModel(x/conf::model_size_div, y/conf::model_size_div, tile.height, radius/conf::model_size_div, color);
                    break;

                case 1:   // Hauteur
                    c = tile.height;
                    color = glm::vec3(c, c, c);
                    hex = createTileModel(x/conf::model_size_div, y/conf::model_size_div, tile.height*5, radius/conf::model_size_div, color);
                    break;

                case 2:   // Température
                    c = ((tile.temperature - gameParam::min_temp) / (gameParam::max_temp - gameParam::min_temp));
                    color = glm::vec3(c, c, c);
                    hex = createTileModel(x/conf::model_size_div, y/conf::model_size_div, c*5, radius/conf::model_size_div, color);
                    break;

                case 3:   // Précipitation
                    c = (tile.precipitation / gameParam::max_precipitation);
                    color = glm::vec3(c, c, c);
                    hex = createTileModel(x/conf::model_size_div, y/conf::model_size_div, c*5, radius/conf::model_size_div, color);
                    break;
            }

            initObject(hex);
            map::hexmap_drawable.push_back(hex);
        }
    }
}

void createRivers(Tile& tile) {
    Tile* lowest = lowestNeighbor(tile);
    if (!lowest) return;

    if (lowest->height >= tile.height)
        return; // pas plus bas → fin du fleuve

    lowest->setBiomeAquatic(); // devient rivière

    createRivers(*lowest);
}

Tile* lowestNeighbor(Tile& tile) {
    Tile* lowest = nullptr;

    for (Tile* n : tile.getAllNeighbors()) {
        if (!lowest || n->height < lowest->height) {
            lowest = n;
        }
    }

    if(lowest->biome.biomeType == BiomeType::Water) return nullptr;

    return lowest;
}