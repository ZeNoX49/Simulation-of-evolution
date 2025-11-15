#include <cmath>
#include "map.hpp"
#include "../game.hpp"
#include "../configuration.hpp"
#include "../utils/perlinNoise.hpp"
#include "../environment/hexCoord.hpp"
#include "../utils/mathUtils.hpp"

namespace map {
    std::unordered_map<HexCoord, Tile> hexmap;
    std::vector<ObjData> hexmap_drawable;
    // std::vector<sf::Text> text_drawable;
    // sf::Font font;
}

void createHexmap() {
    map::hexmap.clear();
    map::hexmap_drawable.clear();
    // map::hexmap_drawable.setPrimitiveType(sf::PrimitiveType::Triangles);
    // map::text_drawable.clear();

    // Calcul du radius et des dimensions
    const float totalHeightFactor = 1.0f + (game::map_size - 1) * 0.75f;
    const float h = conf::game_window_height_f / totalHeightFactor;
    const float radius = h / 2.0f;
    const float w = std::sqrt(3.f) * radius;

    const float totalWidth = game::map_size * w;
    const float offsetX = conf::offsetX + (conf::game_window_width_f - totalWidth) / 2.0f + radius * std::sqrt(3.f) / 2.0f;
    const float offsetY = conf::offsetY + radius;

    float mult = 50.0f;

    // Centrer la caméra
    float centerX = totalWidth / 2.0f / mult;
    float centerZ = ((game::map_size - 1) * (h * 0.75f)) / 2.0f / mult;
    game::cam.target = glm::vec3(centerX, 0.0f, centerZ);

    /* ----- 1. hauteur et biome d'eau ----- */
    PerlinNoise heightPerlin(game::water_seed);

    for (int row = 0; row < game::map_size; ++row) {
        int colsInRow = (row % 2 == 0) ? game::map_size : game::map_size - 1;

        for (int col = 0; col < colsInRow; ++col) {
            float gridX = static_cast<float>(col) + ((row % 2) ? 0.5f : 0.0f);
            float gridY = static_cast<float>(row);

            float height = heightPerlin.normalized2D(
                gridX * game::water_frequency,
                gridY * game::water_frequency,
                game::water_octaves,
                game::water_persistence,
                game::water_lacunarity,
                1.0
            );

            Tile tile;
            HexCoord hc;
            hc.x = gridX;
            hc.y = gridY;
            tile.hexCoord = hc;
            tile.height = MathUtils::clamp(height, 0.0f, 1.0f);
            
            if (tile.height < game::water_threshold) {
                tile.setBiomeAquatic();
            }

            map::hexmap[hc] = tile;
        }
    }

    /* ----- 2. biomes ----- */
    for (auto& [hc, tile] : map::hexmap) {
        if(tile.height >= game::water_threshold) {
            int nbAquaticNeighbors = 0;
            for (auto* neighbor : tile.getAllNeighbors()) {
                if(neighbor && neighbor->height < game::water_threshold) {
                    nbAquaticNeighbors++;
                }
            }
            tile.define_biome(nbAquaticNeighbors);
        }
    }

    // if (!map::font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
    //     // Gérer l'erreur si la font ne charge pas
    //     return;
    // }

    /* ----- 3. hexagones ----- */
    for (int row = 0; row < game::map_size; ++row) {
        int colsInRow = (row % 2 == 0) ? game::map_size : game::map_size - 1;

        for (int col = 0; col < colsInRow; ++col) {
            float x = col * w + ((row % 2) ? w / 2.0f : 0.0f);
            float y = row * (h * 0.75f);
            // sf::Vector2f pos(offsetX + x, offsetY + y);

            float gridX = static_cast<float>(col) + ((row % 2) ? 0.5f : 0.0f);
            float gridY = static_cast<float>(row);

            HexCoord hc;
            hc.x = gridX;
            hc.y = gridY;
            Tile& tile = map::hexmap[hc];

            glm::vec3 color;
            float c;

            // sf::Text text{map::font, "", 10};
            // text.setFillColor(sf::Color::Red);

            switch (game::tile_color) {
                case 0:   // Biome
                    color = tile.biome.color;
                    break;

                case 1:   // Hauteur
                    c = tile.height;
                    color = glm::vec3(c, c, c);
                    // text.setString(std::format("{:.2f}", tile.height));
                    break;

                case 2:   // Température
                    c = ((tile.temperature + 40.0f) / 70.0f);
                    color = glm::vec3(c, c, c);
                    // text.setString(std::format("{:.2f}", tile.temperature));
                    break;

                case 3:   // Précipitation
                    c = (tile.precipitation / 325.0f);
                    color = glm::vec3(c, c, c);
                    // text.setString(std::format("{:.2f}", tile.precipitation));
                    break;
            }

            // // Récupérer les dimensions du texte
            // sf::FloatRect textBounds = text.getLocalBounds();
            // // Centrer le texte sur la position de l'hexagone
            // text.setOrigin({textBounds.position.x + textBounds.size.x / 2.0f, 
            //                 textBounds.position.y + textBounds.size.y / 2.0f});
            // text.setPosition(pos);  // pos est déjà le centre de l'hexagone

            // map::text_drawable.push_back(text);

            ObjData hex = createTileModel(x/mult, y/mult, tile.height, radius/mult, color);
            initObject(hex);
            map::hexmap_drawable.push_back(hex);

            // const int sides = 6;
            // const float rotationDeg = 90.f;
            // const float rotationRad = rotationDeg * 3.14159265f / 180.f;
            // sf::Vertex center(pos, color);
            // for (int i = 0; i < sides; ++i) {
            //     float angle1 = (i * 60.f) * 3.14159265f / 180.f + rotationRad;
            //     float angle2 = ((i + 1) * 60.f) * 3.14159265f / 180.f + rotationRad;

            //     sf::Vector2f p1(pos.x + radius * std::cos(angle1), pos.y + radius * std::sin(angle1));
            //     sf::Vector2f p2(pos.x + radius * std::cos(angle2), pos.y + radius * std::sin(angle2));

            //     // map::hexmap_drawable.append(center);
            //     // map::hexmap_drawable.append(sf::Vertex(p1, color));
            //     // map::hexmap_drawable.append(sf::Vertex(p2, color));
            // }
        }
    }
}