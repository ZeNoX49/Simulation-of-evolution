#include <cmath>
#include "map.hpp"
#include "tile.hpp"
#include "../game.hpp"
#include "../configuration.hpp"
#include "../utils/perlinNoise.hpp"
#include "../environment/hexCoord.hpp"
#include "../environment/riverGenerator.hpp"

// Variables globales pour la simulation
namespace map {
    std::vector<sf::ConvexShape> hexmap;
    
    // Nouvelles données pour la simulation
    std::unordered_map<HexCoord, float> heightMap;
    std::unordered_map<HexCoord, std::string> biomeMap;
    std::unordered_map<HexCoord, Biome> fullBiomeData;
    std::unordered_map<HexCoord, float> waterDistances;
    std::unordered_set<HexCoord> riverTiles;
}

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
    
    // Nettoyer les données précédentes
    map::heightMap.clear();
    map::biomeMap.clear();
    map::fullBiomeData.clear();
    map::waterDistances.clear();
    map::riverTiles.clear();
    
    // 1. Générer la heightmap avec Perlin Noise
    PerlinNoise heightPerlin(game::water_seed);
    
    for (int row = 0; row < game::map_size; ++row) {
        int colsInRow = (row % 2 == 0) ? game::map_size : game::map_size - 1;
        
        for (int col = 0; col < colsInRow; ++col) {
            // Coordonnées de grille
            float gridX = static_cast<float>(col) + ((row % 2) ? 0.5f : 0.0f);
            float gridY = static_cast<float>(row);
            
            // Convertir en HexCoord (cube coordinates)
            HexCoord hexCoord = HexCoord::fromOffset(col, row);
            
            // Générer la hauteur
            float height = heightPerlin.normalized2D(
                gridX * game::water_frequency,
                gridY * game::water_frequency,
                game::water_octaves,
                game::water_persistence,
                game::water_lacunarity,
                1.0
            );
            
            // Stocker dans heightMap
            map::heightMap[hexCoord] = height;
        }
    }
    
    // 2. Générer les rivières
    RiverGenerator riverGen(map::heightMap, game::map_size);
    auto riverData = riverGen.generateRivers(5, 10);  // 5 sources, min 10 tiles
    map::riverTiles = riverData.riverTiles;
    map::waterDistances = riverData.distanceToRiver;
    
    // 3. Générer les biomes et créer les hexagones visuels
    for (int row = 0; row < game::map_size; ++row) {
        int colsInRow = (row % 2 == 0) ? game::map_size : game::map_size - 1;
        
        for (int col = 0; col < colsInRow; ++col) {
            // Coordonnées écran
            float x = col * w + ((row % 2) ? w / 2.0f : 0.0f);
            float y = row * (h * 0.75f);
            sf::Vector2f pos(offsetX + x, offsetY + y);
            
            // Coordonnées de grille
            float gridX = static_cast<float>(col) + ((row % 2) ? 0.5f : 0.0f);
            float gridY = static_cast<float>(row);
            
            HexCoord hexCoord = HexCoord::fromOffset(col, row);
            
            // Obtenir la hauteur
            float height = map::heightMap[hexCoord];
            
            // Définir le biome
            Biome biome;
            sf::Color color;
            
            // Si rivière, biome aquatique
            if (map::riverTiles.count(hexCoord)) {
                float temperature = compute_temperature(gridX, gridY, height);
                float precipitation = compute_precipitation(gridX, gridY, height, temperature);
                biome = Biome{"Aquatique", sf::Color(0, 100, 200), temperature, precipitation};
                color = biome.color;
            }
            // Si eau (hauteur basse)
            else if (height < game::water_threshold) {
                float temperature = compute_temperature(gridX, gridY, height);
                float precipitation = compute_precipitation(gridX, gridY, height, temperature);
                biome = Biome{"Aquatique", sf::Color(0, 0, 220), temperature, precipitation};
                color = biome.color;
            }
            // Sinon, biome terrestre
            else {
                biome = define_biome(gridX, gridY, height);
                color = biome.color;
            }
            
            // Stocker les données du biome
            map::biomeMap[hexCoord] = biome.name;
            map::fullBiomeData[hexCoord] = biome;
            
            // Créer l'hexagone visuel
            hm.push_back(createHexagon(pos, radius, color));
        }
    }
    
    map::hexmap = hm;
}

// Fonction pour obtenir les données nécessaires à la simulation
SimulationMapData getSimulationData() {
    SimulationMapData data;
    data.heightMap = map::heightMap;
    data.biomeMap = map::biomeMap;
    data.waterDistances = map::waterDistances;
    return data;
}