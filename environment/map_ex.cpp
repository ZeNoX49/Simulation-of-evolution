// #include <cmath>
// #include "map.hpp"
// #include "../gameParam.hpp"
// #include "../configuration.hpp"
// #include "../utils/perlinNoise.hpp"
// #include "../environment/hexCoord.hpp"
// #include "../utils/mathUtils.hpp"
// #include "../object/tileModel.hpp"

// float mult = 50.0f;
// PerlinNoise heightPerlin = PerlinNoise(gameParam::water_seed);

// void createHexmap() {
//     map::hexmap.clear();
//     map::hexmap_drawable.clear();

//     // Calcul du radius et des dimensions
//     const float totalHeightFactor = 1.0f + (gameParam::map_size - 1) * 0.75f;
//     const float h = conf::game_window_height_f / totalHeightFactor;
//     const float radius = h / 2.0f;
//     const float w = std::sqrt(3.f) * radius;

//     const float totalWidth = gameParam::map_size * w;
//     const float offsetX = conf::offsetX + (conf::game_window_width_f - totalWidth) / 2.0f + radius * std::sqrt(3.f) / 2.0f;
//     const float offsetY = conf::offsetY + radius;

//     // Centrer la caméra
//     float centerX = totalWidth / 2.0f / mult;
//     float centerZ = ((gameParam::map_size - 1) * (h * 0.75f)) / 2.0f / mult;
//     gameCam::cam.target = glm::vec3(centerX, 0.0f, centerZ);

//     /* ----- 1. hauteur et biome d'eau ----- */
//     std::vector<float> noiseSeed;
//     noiseSeed.resize(gameParam::map_size * gameParam::map_size);
//     for(int i = 0; i < gameParam::map_size * gameParam::map_size; i++) noiseSeed[i] = (float)rand() / (float)RAND_MAX;

//     heightPerlin.setSeed(gameParam::water_seed);

//     for (int row = 0; row < gameParam::map_size; ++row) {
//         int colsInRow = (row % 2 == 0) ? gameParam::map_size : gameParam::map_size - 1;

//         for (int col = 0; col < colsInRow; ++col) {
//             float gridX = static_cast<float>(col) + ((row % 2) ? 0.5f : 0.0f);
//             float gridY = static_cast<float>(row);

//             float height = heightPerlin.fractalNoise(
//                 gridX * gameParam::water_frequency,
//                 gridY * gameParam::water_frequency,
//                 gameParam::water_octaves,
//                 gameParam::water_persistence,
//                 gameParam::water_lacunarity
//             );

//             Tile tile;
//             HexCoord hc;
//             hc.x = gridX;
//             hc.y = gridY;
//             tile.hexCoord = hc;
//             tile.height = MathUtils::clamp(height, 0.0f, 1.0f);
            
//             if (tile.height < gameParam::water_threshold) {
//                 tile.setBiomeAquatic();
//             }

//             map::hexmap[hc] = tile;
//         }
//     }
    

//     /* ----- 2. biomes ----- */
//     for (auto& [hc, tile] : map::hexmap) {
//         if(tile.height >= gameParam::water_threshold) {
//             int nbAquaticNeighbors = 0;
//             for (auto* neighbor : tile.getAllNeighbors()) {
//                 if(neighbor && neighbor->height < gameParam::water_threshold) {
//                     nbAquaticNeighbors++;
//                 }
//             }
//             tile.define_biome(nbAquaticNeighbors);
//         }
//     }

//     /* ----- 3. hexagones ----- */
//     for (int row = 0; row < gameParam::map_size; ++row) {
//         int colsInRow = (row % 2 == 0) ? gameParam::map_size : gameParam::map_size - 1;

//         for (int col = 0; col < colsInRow; ++col) {
//             float x = col * w + ((row % 2) ? w / 2.0f : 0.0f);
//             float y = row * (h * 0.75f);
//             // sf::Vector2f pos(offsetX + x, offsetY + y);

//             float gridX = static_cast<float>(col) + ((row % 2) ? 0.5f : 0.0f);
//             float gridY = static_cast<float>(row);

//             HexCoord hc;
//             hc.x = gridX;
//             hc.y = gridY;
//             Tile& tile = map::hexmap[hc];

//             glm::vec3 color;
//             float c;

//             ObjData hex;

//             switch (gameParam::tile_color) {
//                 case 0:   // Biome
//                     color = tile.biome.color;
//                     hex = createTileModel(x/mult, y/mult, tile.height, radius/mult, color);
//                     break;

//                 case 1:   // Hauteur
//                     c = tile.height;
//                     color = glm::vec3(c, c, c);
//                     hex = createTileModel(x/mult, y/mult, tile.height*5, radius/mult, color);
//                     break;

//                 case 2:   // Température
//                     c = ((tile.temperature + 40.0f) / 70.0f);
//                     color = glm::vec3(c, c, c);
//                     hex = createTileModel(x/mult, y/mult, c*5, radius/mult, color);
//                     break;

//                 case 3:   // Précipitation
//                     c = (tile.precipitation / 325.0f);
//                     color = glm::vec3(c, c, c);
//                     hex = createTileModel(x/mult, y/mult, c*5, radius/mult, color);
//                     break;
//             }

//             initObject(hex);
//             map::hexmap_drawable.push_back(hex);
//         }
//     }
// }