#include <cmath>
#include "map.hpp"
#include "../gameParam.hpp"
#include "../configuration.hpp"
#include "../utils/perlinNoise.hpp"
#include "../environment/hexCoord.hpp"
#include "../utils/mathUtils.hpp"
#include "../object/tileModel.hpp"

// Multiplier pour mise à l'échelle
float mult = 50.0f;

// Perlin pour hauteur
PerlinNoise heightPerlin = PerlinNoise(gameParam::map_seed);

void createHexmap(TerrainGenerator& terrainGen) {
    map::hexmap.clear();
    map::hexmap_drawable.clear();
    
    const float totalHeightFactor = 1.0f + (gameParam::map_size - 1) * 0.75f;
    const float h = conf::game_window_height_f / totalHeightFactor;
    const float radius = h / 2.0f;
    const float w = std::sqrt(3.f) * radius;

    const float totalWidth = gameParam::map_size * w;
    const float offsetX = conf::offsetX + (conf::game_window_width_f - totalWidth) / 2.0f + radius * std::sqrt(3.f) / 2.0f;
    const float offsetY = conf::offsetY + radius;

    float centerX = totalWidth / 2.0f / mult;
    float centerZ = ((gameParam::map_size - 1) * (h * 0.75f)) / 2.0f / mult;
    gameUtils::cam.target = glm::vec3(centerX, 0.0f, centerZ);

    // -------- 1️⃣ INIT HEIGHTMAP RECTANGULAIRE --------
    terrainGen.width = gameParam::map_size;
    terrainGen.height = gameParam::map_size;
    terrainGen.heights.resize(gameParam::map_size * gameParam::map_size);

    heightPerlin.InitPerm(gameParam::map_seed);

    for (int y = 0; y < gameParam::map_size; ++y) {
        for (int x = 0; x < gameParam::map_size; ++x) {

            float gx = x + ((y % 2) ? 0.5f : 0.0f);
            float gy = y;

            float hval = heightPerlin.FractalNoise(
                gx * gameParam::map_frequency,
                gy * gameParam::map_frequency,
                gameParam::map_octaves,
                gameParam::map_persistence,
                gameParam::map_lacunarity
            );

            terrainGen.heights[y * gameParam::map_size + x] = MathUtils::clamp(hval, 0.0f, 1.0f);
        }
    }

    // -------- 2️⃣ ERROSION + RIVIERES --------
    terrainGen.ApplyErosion(gameParam::erosion_iterations);
    terrainGen.ComputeFlowAccumulation();
    auto rivers = terrainGen.ExtractRivers(gameParam::riverFlowThreshold);
    terrainGen.CarveRivers(rivers);

    // -------- 3️⃣ CREATION DES HEX - PAS DE TROUS --------
    for (int row = 0; row < gameParam::map_size; ++row) {
        int colsInRow = (row % 2 == 0) ? gameParam::map_size : gameParam::map_size - 1;

        for (int col = 0; col < colsInRow; ++col) {

            float gx = col + ((row % 2) ? 0.5f : 0.0f);
            float gy = row;

            HexCoord hc{gx, gy};
            Tile tile;
            tile.hexCoord = hc;

            int index = row * gameParam::map_size + col;
            tile.height = terrainGen.heights[index];
            tile.flow = terrainGen.flow[index];

            if (tile.height < gameParam::water_threshold || tile.flow > gameParam::riverFlowThreshold) {
                tile.setBiomeAquatic();
            }

            map::hexmap[hc] = tile;
        }
    }

    // -------- 4️⃣ BIOMES RESTANTS --------
    for (auto& [hc, tile] : map::hexmap) {
        if (!tile.isWater) {
            int waterN = 0;
            for (auto* n : tile.getAllNeighbors()) {
                if (n && n->isWater) waterN++;
            }
            tile.define_biome(waterN);
        }
    }

    // -------- 5️⃣ MODELES HEX --------
    for (int row = 0; row < gameParam::map_size; ++row) {
        int colsInRow = (row % 2 == 0) ? gameParam::map_size : gameParam::map_size - 1;

        for (int col = 0; col < colsInRow; ++col) {
            float x = col * w + ((row % 2) ? w / 2.0f : 0.0f);
            float y = row * (h * 0.75f);
            HexCoord hc{col + ((row % 2) ? 0.5f : 0.0f), (float)row};
            Tile& tile = map::hexmap[hc];

            glm::vec3 color;
            float c;

            ObjData hex;

            switch (gameParam::tile_color) {
            case 0:
                color = tile.biome.color;
                hex = createTileModel(x/mult, y/mult, tile.height, radius/mult, color);
                break;
            case 1:
                c = tile.height;
                color = {c, c, c};
                hex = createTileModel(x/mult, y/mult, tile.height*5, radius/mult, color);
                break;
            case 2:
                c = (tile.temperature + 40.0f) / 70.0f;
                color = {c, c, c};
                hex = createTileModel(x/mult, y/mult, c*5, radius/mult, color);
                break;
            case 3:
                c = tile.precipitation / 325.0f;
                color = {c, c, c};
                hex = createTileModel(x/mult, y/mult, c*5, radius/mult, color);
                break;
            }

            initObject(hex);
            map::hexmap_drawable.push_back(hex);
        }
    }
}