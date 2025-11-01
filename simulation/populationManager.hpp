#ifndef POPULATION_MANAGER_HPP
#define POPULATION_MANAGER_HPP

#include "../creatures/creature.hpp"
#include "../environment/hexCoord.hpp"
#include "../utils/random.hpp"
#include <vector>
#include <unordered_map>

/**
 * Gestionnaire des populations de créatures
 * - Spawn initial
 * - Équilibrage des populations
 * - Statistiques démographiques
 */
class PopulationManager {
private:
    int mapSize;
    std::unordered_map<HexCoord, float>& heightMap;
    
public:
    PopulationManager(int mapSize, std::unordered_map<HexCoord, float>& heightMap)
        : mapSize(mapSize), heightMap(heightMap)
    {}
    
    // Spawn initial de créatures
    void spawnInitialPopulation(
        std::vector<Creature>& creatures,
        int herbivoreCount,
        int carnivoreCount,
        int omnivoreCount
    ) {
        // Réinitialiser l'ID counter
        Creature::resetIdCounter();
        
        // Spawn herbivores
        for (int i = 0; i < herbivoreCount; ++i) {
            HexCoord spawnPos = findSuitableSpawnLocation();
            CreatureStats stats = generateHerbivoreStats();
            creatures.emplace_back(spawnPos, stats, 0);
        }
        
        // Spawn carnivores
        for (int i = 0; i < carnivoreCount; ++i) {
            HexCoord spawnPos = findSuitableSpawnLocation();
            CreatureStats stats = generateCarnivoreStats();
            creatures.emplace_back(spawnPos, stats, 0);
        }
        
        // Spawn omnivores
        for (int i = 0; i < omnivoreCount; ++i) {
            HexCoord spawnPos = findSuitableSpawnLocation();
            CreatureStats stats = generateOmnivoreStats();
            creatures.emplace_back(spawnPos, stats, 0);
        }
    }
    
    // Générer des stats pour herbivore
    CreatureStats generateHerbivoreStats() {
        CreatureStats stats;
        
        stats.size = Random::randFloat(8.0f, 15.0f);
        stats.speed = Random::randFloat(8.0f, 15.0f);
        stats.reproductionRate = Random::randFloat(8.0f, 15.0f);
        stats.diet = Random::randInt(-99, -40);  // Herbivore
        stats.stealth = Random::randFloat(5.0f, 20.0f);
        stats.perception = Random::randFloat(8.0f, 15.0f);
        
        stats.clampValues();
        return stats;
    }
    
    // Générer des stats pour carnivore
    CreatureStats generateCarnivoreStats() {
        CreatureStats stats;
        
        stats.size = Random::randFloat(10.0f, 20.0f);  // Plus grands
        stats.speed = Random::randFloat(12.0f, 18.0f);  // Plus rapides
        stats.reproductionRate = Random::randFloat(5.0f, 10.0f);  // Reproduction plus lente
        stats.diet = Random::randInt(40, 99);  // Carnivore
        stats.stealth = Random::randFloat(10.0f, 25.0f);
        stats.perception = Random::randFloat(12.0f, 20.0f);
        
        stats.clampValues();
        return stats;
    }
    
    // Générer des stats pour omnivore
    CreatureStats generateOmnivoreStats() {
        CreatureStats stats;
        
        stats.size = Random::randFloat(8.0f, 15.0f);
        stats.speed = Random::randFloat(10.0f, 15.0f);
        stats.reproductionRate = Random::randFloat(8.0f, 12.0f);
        stats.diet = Random::randInt(-20, 20);  // Omnivore
        stats.stealth = Random::randFloat(8.0f, 18.0f);
        stats.perception = Random::randFloat(10.0f, 18.0f);
        
        stats.clampValues();
        return stats;
    }
    
    // Trouver une position de spawn appropriée (terre, pas trop haute)
    HexCoord findSuitableSpawnLocation() {
        const int maxAttempts = 100;
        
        for (int attempt = 0; attempt < maxAttempts; ++attempt) {
            // Coordonnées aléatoires
            int q = Random::randInt(-mapSize / 2, mapSize / 2);
            int r = Random::randInt(-mapSize / 2, mapSize / 2);
            HexCoord coord(q, r);
            
            // Vérifier si la position existe dans la heightmap
            auto it = heightMap.find(coord);
            if (it == heightMap.end()) continue;
            
            float height = it->second;
            
            // Spawn sur terre (pas eau, pas trop haute)
            if (height > 0.4f && height < 0.8f) {
                return coord;
            }
        }
        
        // Fallback : centre de la carte
        return HexCoord(0, 0);
    }
    
    // Statistiques démographiques
    struct PopulationStats {
        int totalPopulation;
        int herbivores;
        int carnivores;
        int omnivores;
        
        int youngCreatures;   // Age < 50
        int adultCreatures;   // Age 50-500
        int oldCreatures;     // Age > 500
        
        float averageAge;
        float averageGeneration;
        
        int totalGenerations;
        int maxGeneration;
        
        PopulationStats()
            : totalPopulation(0), herbivores(0), carnivores(0), omnivores(0),
              youngCreatures(0), adultCreatures(0), oldCreatures(0),
              averageAge(0.0f), averageGeneration(0.0f),
              totalGenerations(0), maxGeneration(0)
        {}
    };
    
    PopulationStats getPopulationStats(const std::vector<Creature>& creatures) const {
        PopulationStats stats;
        
        int totalAge = 0;
        int totalGeneration = 0;
        
        for (const auto& creature : creatures) {
            if (!creature.isAlive) continue;
            
            stats.totalPopulation++;
            totalAge += creature.age;
            totalGeneration += creature.generation;
            
            // Classification par diet
            if (creature.stats.isHerbivore()) {
                stats.herbivores++;
            } else if (creature.stats.isCarnivore()) {
                stats.carnivores++;
            } else {
                stats.omnivores++;
            }
            
            // Classification par âge
            if (creature.age < 50) {
                stats.youngCreatures++;
            } else if (creature.age < 500) {
                stats.adultCreatures++;
            } else {
                stats.oldCreatures++;
            }
            
            // Génération max
            if (creature.generation > stats.maxGeneration) {
                stats.maxGeneration = creature.generation;
            }
        }
        
        if (stats.totalPopulation > 0) {
            stats.averageAge = static_cast<float>(totalAge) / stats.totalPopulation;
            stats.averageGeneration = static_cast<float>(totalGeneration) / stats.totalPopulation;
        }
        
        stats.totalGenerations = stats.maxGeneration + 1;
        
        return stats;
    }
    
    // Vérifier si la population est en danger d'extinction
    bool isExtinctionRisk(const std::vector<Creature>& creatures, int threshold = 10) const {
        int aliveCount = 0;
        
        for (const auto& creature : creatures) {
            if (creature.isAlive) {
                aliveCount++;
                if (aliveCount >= threshold) {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    // Spawn de secours si extinction imminente
    void emergencySpawn(
        std::vector<Creature>& creatures,
        int count = 10
    ) {
        for (int i = 0; i < count; ++i) {
            HexCoord spawnPos = findSuitableSpawnLocation();
            
            // Spawn équilibré
            CreatureStats stats;
            int type = i % 3;
            
            if (type == 0) {
                stats = generateHerbivoreStats();
            } else if (type == 1) {
                stats = generateCarnivoreStats();
            } else {
                stats = generateOmnivoreStats();
            }
            
            creatures.emplace_back(spawnPos, stats, 0);
        }
    }
    
    // Obtenir la distribution spatiale des créatures
    std::unordered_map<HexCoord, int> getSpatialDistribution(const std::vector<Creature>& creatures) const {
        std::unordered_map<HexCoord, int> distribution;
        
        for (const auto& creature : creatures) {
            if (creature.isAlive) {
                distribution[creature.position]++;
            }
        }
        
        return distribution;
    }
    
    // Vérifier s'il y a surpopulation sur certaines tiles
    bool hasOvercrowding(const std::vector<Creature>& creatures, int threshold = 5) const {
        auto distribution = getSpatialDistribution(creatures);
        
        for (const auto& [coord, count] : distribution) {
            if (count > threshold) {
                return true;
            }
        }
        
        return false;
    }
};

#endif // POPULATION_MANAGER_HPP