#ifndef MAP_HPP
#define MAP_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "../environment/hexCoord.hpp"
#include "tile.hpp"

// Données pour la simulation
struct SimulationMapData {
    std::unordered_map<HexCoord, float> heightMap;
    std::unordered_map<HexCoord, std::string> biomeMap;
    std::unordered_map<HexCoord, float> waterDistances;
};

namespace map {
    extern std::vector<sf::ConvexShape> hexmap;
    
    // Nouvelles données pour la simulation
    extern std::unordered_map<HexCoord, float> heightMap;
    extern std::unordered_map<HexCoord, std::string> biomeMap;
    extern std::unordered_map<HexCoord, Biome> fullBiomeData;
    extern std::unordered_map<HexCoord, float> waterDistances;
    extern std::unordered_set<HexCoord> riverTiles;
}

// Créer un hexagone graphique
sf::ConvexShape createHexagon(sf::Vector2f center, float radius, sf::Color color);

// Générer la carte complète
void createHexmap();

// Obtenir les données pour la simulation
SimulationMapData getSimulationData();

#endif // MAP_HPP