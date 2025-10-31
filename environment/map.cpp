#include <cmath>

#include "map.hpp"
#include "tile.hpp"
#include "../game.hpp"
#include "../configuration.hpp"
#include "../util/perlinNoise.hpp"

sf::ConvexShape createHexagon(sf::Vector2f center, float radius, sf::Color color) {
    sf::ConvexShape hex;
    hex.setPointCount(6);

    for (int i = 0; i < 6; ++i) {
        float angle = std::numbers::pi / 6 + i * std::numbers::pi / 3;
        hex.setPoint(i, {
            radius * std::cos(angle),
            radius * std::sin(angle)
        });
    }

    hex.setPosition(center);
    hex.setFillColor(color);
    hex.setOutlineThickness(1);
    hex.setOutlineColor(sf::Color::Black);
    return hex;
}

void createHexmap() {
    std::vector<sf::ConvexShape> hm;

    // Calcul correct du radius
    const float totalHeightFactor = 1.0f + (game::map_size - 1) * 0.75f;
    const float h = conf::game_window_size.y / totalHeightFactor;
    const float radius = h / 2.0f;
    const float w = std::sqrt(3.f) * radius;

    // Calcul de la largeur totale de la grille
    const float totalWidth = game::map_size * w;

    // Centrage horizontal automatique
    const float offsetX = conf::offsetX + (conf::game_window_size.x - totalWidth) / 2.0f + radius * std::sqrt(3.f) / 2.0f;
    const float offsetY = conf::offsetY + radius;

    PerlinNoise perlin(game::water_seed);
    
    for (int row = 0; row < game::map_size; ++row) {
        int colsInRow = (row % 2 == 0) ? game::map_size : game::map_size - 1;
        
        for (int col = 0; col < colsInRow; ++col) {
            // Coordonnées écran
            float x = col * w + ((row % 2) ? w / 2.0f : 0.0f);
            float y = row * (h * 0.75f);
            sf::Vector2f pos(offsetX + x, offsetY + y);

            // IMPORTANT : Utiliser les coordonnées de grille (col, row) 
            // et non les coordonnées écran (x, y) pour le Perlin Noise
            float gridX = static_cast<float>(col) + ((row % 2) ? 0.5f : 0.0f);
            float gridY = static_cast<float>(row);

            // Obtenir la valeur de hauteur du Perlin Noise
            float height = perlin.normalized2D(
                gridX * game::water_frequency,  // Multiplier par frequency ici
                gridY * game::water_frequency,
                game::water_octaves,
                game::water_persistence,
                game::water_lacunarity,
                1.0  // Frequency = 1.0 dans la fonction
            );

            Tile tile;
            if (height < game::water_threshold) {
                float temperature = compute_temperature(gridX, gridY, height);
                float precipitation = compute_precipitation(gridX, gridY, height, temperature);
                tile = Tile{gridX, gridY, height, Biome{"Aquatique", sf::Color(0,0,220), temperature, precipitation}};
            } else {
                tile = Tile{gridX, gridY, height, define_biome(gridX, gridY, height)};
            }

            sf::Color color = tile.biome.color;

            hm.push_back(createHexagon(pos, radius, color));
        }
    }

    map::hexmap = hm;
}