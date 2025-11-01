#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include "turnManager.hpp"
#include "populationManager.hpp"
#include "../creatures/creature.hpp"
#include "../creatures/scent.hpp"
#include "../environment/hexCoord.hpp"
#include "../environment/tileResources.hpp"
#include "../systems/resourceSystem.hpp"
#include <vector>
#include <unordered_map>

/**
 * Classe principale de simulation
 * Orchestre tout : créatures, ressources, turns
 */
class Simulation {
private:
    // Map et ressources
    int mapSize;
    std::unordered_map<HexCoord, float> heightMap;
    std::unordered_map<HexCoord, TileResources> tileResources;
    
    // Systèmes
    ResourceSystem resourceSystem;
    ScentManager scentManager;
    TurnManager* turnManager;
    PopulationManager* populationManager;
    
    // Créatures
    std::vector<Creature> creatures;
    
    // État
    bool initialized;
    bool paused;
    
public:
    Simulation(int mapSize)
        : mapSize(mapSize),
          resourceSystem(tileResources),
          initialized(false),
          paused(false)
    {
        turnManager = new TurnManager(resourceSystem, tileResources, scentManager);
        populationManager = new PopulationManager(mapSize, heightMap);
    }
    
    ~Simulation() {
        delete turnManager;
        delete populationManager;
    }
    
    // Initialiser la simulation
    void initialize(
        const std::unordered_map<HexCoord, float>& heights,
        const std::unordered_map<HexCoord, std::string>& biomes,
        const std::unordered_map<HexCoord, float>& waterDistances,
        int herbivoreCount = 20,
        int carnivoreCount = 10,
        int omnivoreCount = 15
    ) {
        // Copier la heightmap
        heightMap = heights;
        
        // Initialiser les ressources des tiles selon les biomes
        for (const auto& [coord, biomeName] : biomes) {
            float waterDist = 999.0f;
            auto it = waterDistances.find(coord);
            if (it != waterDistances.end()) {
                waterDist = it->second;
            }
            
            resourceSystem.initializeTileResources(coord, biomeName, waterDist);
        }
        
        // Spawn initial de créatures
        populationManager->spawnInitialPopulation(
            creatures,
            herbivoreCount,
            carnivoreCount,
            omnivoreCount
        );
        
        initialized = true;
    }
    
    // Exécuter un tour
    void runTurn() {
        if (!initialized || paused) return;
        
        turnManager->executeTurn(creatures);
        
        // Vérifier extinction et spawn de secours si nécessaire
        if (populationManager->isExtinctionRisk(creatures, 5)) {
            populationManager->emergencySpawn(creatures, 10);
        }
    }
    
    // Exécuter N tours
    void runNTurns(int n) {
        for (int i = 0; i < n; ++i) {
            runTurn();
        }
    }
    
    // Pause/Resume
    void pause() { paused = true; }
    void resume() { paused = false; }
    void togglePause() { paused = !paused; }
    bool isPaused() const { return paused; }
    
    // Reset
    void reset() {
        creatures.clear();
        scentManager.clear();
        turnManager->resetTurn();
        initialized = false;
    }
    
    // Getters
    const std::vector<Creature>& getCreatures() const { return creatures; }
    std::vector<Creature>& getCreatures() { return creatures; }
    
    const ScentManager& getScentManager() const { return scentManager; }
    ScentManager& getScentManager() { return scentManager; }
    
    const std::unordered_map<HexCoord, TileResources>& getTileResources() const {
        return tileResources;
    }
    
    int getCurrentTurn() const {
        return turnManager->getCurrentTurn();
    }
    
    int getMapSize() const { return mapSize; }
    
    // Statistiques globales
    struct SimulationStats {
        int currentTurn;
        PopulationManager::PopulationStats population;
        TurnManager::TurnStats turnStats;
        ResourceSystem::GlobalResourceStats resources;
        int totalScents;
        
        SimulationStats()
            : currentTurn(0), totalScents(0)
        {}
    };
    
    SimulationStats getStats() const {
        SimulationStats stats;
        
        stats.currentTurn = turnManager->getCurrentTurn();
        stats.population = populationManager->getPopulationStats(creatures);
        stats.turnStats = turnManager->getTurnStats(creatures);
        stats.resources = resourceSystem.getGlobalStats();
        stats.totalScents = scentManager.getCount();
        
        return stats;
    }
    
    // Afficher les stats dans la console (debug)
    void printStats() const {
        auto stats = getStats();
        
        std::cout << "\n=== SIMULATION STATS (Turn " << stats.currentTurn << ") ===" << std::endl;
        
        std::cout << "\nPopulation:" << std::endl;
        std::cout << "  Total: " << stats.population.totalPopulation << std::endl;
        std::cout << "  Herbivores: " << stats.population.herbivores << std::endl;
        std::cout << "  Carnivores: " << stats.population.carnivores << std::endl;
        std::cout << "  Omnivores: " << stats.population.omnivores << std::endl;
        std::cout << "  Average Age: " << stats.population.averageAge << std::endl;
        std::cout << "  Max Generation: " << stats.population.maxGeneration << std::endl;
        
        std::cout << "\nTurn Activity:" << std::endl;
        std::cout << "  Moving: " << stats.turnStats.movingCreatures << std::endl;
        std::cout << "  Hungry: " << stats.turnStats.hungryCreatures << std::endl;
        std::cout << "  Thirsty: " << stats.turnStats.thirstyCreatures << std::endl;
        std::cout << "  Mating: " << stats.turnStats.matingCreatures << std::endl;
        
        std::cout << "\nResources:" << std::endl;
        std::cout << "  Avg PlantFood: " << stats.resources.averagePlantFood << std::endl;
        std::cout << "  Avg Meat: " << stats.resources.averageMeat << std::endl;
        std::cout << "  Tiles with Water: " << stats.resources.tilesWithWater << std::endl;
        
        std::cout << "\nScents: " << stats.totalScents << std::endl;
        
        std::cout << "================================\n" << std::endl;
    }
    
    // Sauvegarder l'état (optionnel, pour reprendre plus tard)
    // TODO: Implémenter serialization si nécessaire
};

#endif // SIMULATION_HPP