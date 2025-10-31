#ifndef RIVER_GENERATOR_HPP
#define RIVER_GENERATOR_HPP

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include "hexCoord.hpp"
#include "../utils/random.hpp"

/**
 * Générateur de rivières basé sur la hauteur du terrain
 * Les rivières coulent du haut vers le bas (downhill)
 */
class RiverGenerator {
public:
    struct RiverData {
        std::unordered_set<HexCoord> riverTiles;  // Tiles contenant de l'eau
        std::unordered_map<HexCoord, float> distanceToRiver;  // Distance de chaque tile à la rivière la plus proche
    };
    
private:
    const std::unordered_map<HexCoord, float>& heightMap;
    int mapSize;
    
public:
    RiverGenerator(const std::unordered_map<HexCoord, float>& heightMap, int mapSize)
        : heightMap(heightMap), mapSize(mapSize)
    {}
    
    // Générer des rivières
    RiverData generateRivers(int numSources = 5, int minRiverLength = 10) {
        RiverData riverData;
        
        // 1. Trouver des sources potentielles (points hauts)
        std::vector<HexCoord> sources = findPotentialSources(numSources);
        
        // 2. Pour chaque source, tracer une rivière vers le bas
        for (const auto& source : sources) {
            traceRiver(source, riverData.riverTiles, minRiverLength);
        }
        
        // 3. Calculer les distances à la rivière pour toutes les tiles
        riverData.distanceToRiver = calculateDistancesToRiver(riverData.riverTiles);
        
        return riverData;
    }
    
private:
    // Trouver des sources potentielles (points hauts, loin des bords)
    std::vector<HexCoord> findPotentialSources(int count) {
        std::vector<std::pair<HexCoord, float>> candidates;
        
        // Parcourir toutes les tiles
        for (const auto& [coord, height] : heightMap) {
            // Ignorer les tiles trop basses (eau)
            if (height < 0.45f) continue;
            
            // Ignorer les tiles trop près des bords
            if (std::abs(coord.q) > mapSize / 2 - 3 || 
                std::abs(coord.r) > mapSize / 2 - 3) {
                continue;
            }
            
            // Vérifier si c'est un point haut local
            bool isLocalHigh = true;
            for (const auto& neighbor : coord.neighbors()) {
                auto it = heightMap.find(neighbor);
                if (it != heightMap.end() && it->second > height) {
                    isLocalHigh = false;
                    break;
                }
            }
            
            if (isLocalHigh && height > 0.5f) {
                candidates.push_back({coord, height});
            }
        }
        
        // Trier par hauteur (du plus haut au plus bas)
        std::sort(candidates.begin(), candidates.end(), 
            [](const auto& a, const auto& b) { return a.second > b.second; });
        
        // Prendre les 'count' plus hauts
        std::vector<HexCoord> sources;
        for (int i = 0; i < std::min(count, static_cast<int>(candidates.size())); ++i) {
            sources.push_back(candidates[i].first);
        }
        
        return sources;
    }
    
    // Tracer une rivière depuis une source
    void traceRiver(HexCoord current, std::unordered_set<HexCoord>& riverTiles, int minLength) {
        std::unordered_set<HexCoord> visited;
        int length = 0;
        
        while (true) {
            // Marquer comme visité
            visited.insert(current);
            
            // Obtenir la hauteur actuelle
            auto currentIt = heightMap.find(current);
            if (currentIt == heightMap.end()) break;
            float currentHeight = currentIt->second;
            
            // Si on atteint l'eau ou le bord, on s'arrête
            if (currentHeight < 0.4f) {
                // C'est de l'eau, on a réussi !
                if (length >= minLength) {
                    // Ajouter toute la rivière
                    for (const auto& tile : visited) {
                        riverTiles.insert(tile);
                    }
                }
                break;
            }
            
            // Trouver le voisin le plus bas
            HexCoord lowestNeighbor = current;
            float lowestHeight = currentHeight;
            
            std::vector<HexCoord> neighbors = current.neighbors();
            
            // Mélanger pour éviter les biais
            Random::shuffle(neighbors);
            
            for (const auto& neighbor : neighbors) {
                // Éviter de revisiter
                if (visited.count(neighbor)) continue;
                
                auto neighborIt = heightMap.find(neighbor);
                if (neighborIt == heightMap.end()) continue;
                
                float neighborHeight = neighborIt->second;
                
                if (neighborHeight < lowestHeight) {
                    lowestHeight = neighborHeight;
                    lowestNeighbor = neighbor;
                }
            }
            
            // Si on n'a pas trouvé de voisin plus bas, on s'arrête
            if (lowestNeighbor == current) {
                break;
            }
            
            // Avec une petite probabilité, ajouter un méandre (bifurcation)
            if (length > 5 && Random::randBool(0.1f)) {
                // Choisir un voisin aléatoire qui descend aussi
                for (const auto& neighbor : neighbors) {
                    if (visited.count(neighbor)) continue;
                    
                    auto neighborIt = heightMap.find(neighbor);
                    if (neighborIt == heightMap.end()) continue;
                    
                    if (neighborIt->second < currentHeight) {
                        // Tracer une branche secondaire
                        traceRiver(neighbor, riverTiles, minLength / 2);
                        break;
                    }
                }
            }
            
            // Continuer vers le voisin le plus bas
            current = lowestNeighbor;
            length++;
            
            // Limite de sécurité
            if (length > mapSize * 2) break;
        }
    }
    
    // Calculer la distance de chaque tile à la rivière la plus proche (BFS)
    std::unordered_map<HexCoord, float> calculateDistancesToRiver(
        const std::unordered_set<HexCoord>& riverTiles
    ) {
        std::unordered_map<HexCoord, float> distances;
        std::queue<std::pair<HexCoord, int>> queue;
        std::unordered_set<HexCoord> visited;
        
        // Initialiser avec les tiles de rivière (distance 0)
        for (const auto& riverTile : riverTiles) {
            queue.push({riverTile, 0});
            visited.insert(riverTile);
            distances[riverTile] = 0.0f;
        }
        
        // BFS pour propager les distances
        while (!queue.empty()) {
            auto [current, dist] = queue.front();
            queue.pop();
            
            // Limiter la propagation (pas besoin de calculer très loin)
            if (dist > 15) continue;
            
            for (const auto& neighbor : current.neighbors()) {
                // Si déjà visité, ignorer
                if (visited.count(neighbor)) continue;
                
                // Si la tile existe dans la heightmap
                if (heightMap.count(neighbor)) {
                    visited.insert(neighbor);
                    distances[neighbor] = static_cast<float>(dist + 1);
                    queue.push({neighbor, dist + 1});
                }
            }
        }
        
        // Pour les tiles non visitées, mettre une grande distance
        for (const auto& [coord, height] : heightMap) {
            if (distances.find(coord) == distances.end()) {
                distances[coord] = 999.0f;  // Très loin
            }
        }
        
        return distances;
    }
};

#endif // RIVER_GENERATOR_HPP