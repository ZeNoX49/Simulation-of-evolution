#ifndef RESOURCE_SYSTEM_HPP
#define RESOURCE_SYSTEM_HPP

#include <unordered_map>
#include "../environment/hexCoord.hpp"
#include "../environment/tileResources.hpp"

/**
 * Système de gestion des ressources de toutes les tiles
 * - Croissance de plantFood
 * - Pourriture de meat
 * - Régénération d'eau
 */
class ResourceSystem {
private:
    // Map des ressources par coordonnée
    std::unordered_map<HexCoord, TileResources>& tileResources;
    
public:
    ResourceSystem(std::unordered_map<HexCoord, TileResources>& resources)
        : tileResources(resources)
    {}
    
    // Mettre à jour toutes les ressources pour un tour
    void update() {
        for (auto& [coord, resources] : tileResources) {
            resources.growPlantFood();
            resources.decayMeat();
        }
    }
    
    // Initialiser les ressources d'une tile selon son biome
    void initializeTileResources(const HexCoord& coord, const std::string& biomeName, float waterDistance) {
        TileResources resources = createResourcesForBiome(biomeName, waterDistance);
        tileResources[coord] = resources;
    }
    
    // Créer des ressources appropriées selon le biome
    TileResources createResourcesForBiome(const std::string& biomeName, float waterDistance) {
        TileResources resources;
        
        // Paramètres par défaut
        float capacity = 50.0f;
        float growthRate = 2.0f;
        float waterAvailability = calculateWaterAvailability(waterDistance);
        
        // Ajuster selon le biome
        if (biomeName == "Forêt tropicale humide" || biomeName == "Forêt tropicale") {
            capacity = 150.0f;
            growthRate = 8.0f;
        }
        else if (biomeName == "Savane") {
            capacity = 80.0f;
            growthRate = 5.0f;
        }
        else if (biomeName == "Forêt pluviale tempérée" || biomeName == "Forêt décidue") {
            capacity = 120.0f;
            growthRate = 6.0f;
        }
        else if (biomeName == "Prairie") {
            capacity = 100.0f;
            growthRate = 7.0f;
        }
        else if (biomeName == "Taïga") {
            capacity = 60.0f;
            growthRate = 3.0f;
        }
        else if (biomeName == "Désert chaud" || biomeName == "Désert froid") {
            capacity = 20.0f;
            growthRate = 1.0f;
        }
        else if (biomeName == "Toundra") {
            capacity = 30.0f;
            growthRate = 1.5f;
        }
        else if (biomeName == "Polaire") {
            capacity = 10.0f;
            growthRate = 0.5f;
        }
        else if (biomeName == "Aquatique") {
            capacity = 0.0f;
            growthRate = 0.0f;
            waterAvailability = INFINITY;  // Eau illimitée
        }
        
        resources.plantCapacity = capacity;
        resources.plantGrowthRate = growthRate;
        resources.water = waterAvailability;
        resources.plantFood = capacity * 0.5f;  // Commence à 50%
        
        return resources;
    }
    
    // Calculer la disponibilité en eau selon la distance à une rivière
    float calculateWaterAvailability(float distanceToRiver) {
        if (distanceToRiver == 0.0f) {
            return INFINITY;  // Sur une rivière = eau illimitée
        }
        else if (distanceToRiver <= 1.0f) {
            return 100.0f;  // Très proche = beaucoup d'eau
        }
        else if (distanceToRiver <= 3.0f) {
            return 50.0f;   // Proche = eau modérée
        }
        else if (distanceToRiver <= 5.0f) {
            return 20.0f;   // Moyen = peu d'eau
        }
        else {
            return 5.0f;    // Loin = très peu d'eau
        }
    }
    
    // Obtenir les ressources d'une tile
    TileResources* getResources(const HexCoord& coord) {
        auto it = tileResources.find(coord);
        if (it != tileResources.end()) {
            return &(it->second);
        }
        return nullptr;
    }
    
    const TileResources* getResources(const HexCoord& coord) const {
        auto it = tileResources.find(coord);
        if (it != tileResources.end()) {
            return &(it->second);
        }
        return nullptr;
    }
    
    // Ajouter de la viande sur une tile (mort d'une créature)
    void addMeatToTile(const HexCoord& coord, float amount) {
        auto* resources = getResources(coord);
        if (resources) {
            resources->addMeat(amount);
        }
    }
    
    // Consommer des ressources sur une tile
    float consumePlantFood(const HexCoord& coord, float amount) {
        auto* resources = getResources(coord);
        return resources ? resources->consumePlantFood(amount) : 0.0f;
    }
    
    float consumeMeat(const HexCoord& coord, float amount) {
        auto* resources = getResources(coord);
        return resources ? resources->consumeMeat(amount) : 0.0f;
    }
    
    float consumeWater(const HexCoord& coord, float amount) {
        auto* resources = getResources(coord);
        return resources ? resources->consumeWater(amount) : 0.0f;
    }
    
    // Vérifier la disponibilité de ressources
    bool hasPlantFood(const HexCoord& coord) const {
        const auto* resources = getResources(coord);
        return resources && resources->hasPlantFood();
    }
    
    bool hasMeat(const HexCoord& coord) const {
        const auto* resources = getResources(coord);
        return resources && resources->hasMeat();
    }
    
    bool hasWater(const HexCoord& coord) const {
        const auto* resources = getResources(coord);
        return resources && resources->hasWater();
    }
    
    // Reset toutes les ressources
    void resetAllResources() {
        for (auto& [coord, resources] : tileResources) {
            resources.resetToInitial();
        }
    }
    
    // Obtenir des statistiques globales
    struct GlobalResourceStats {
        float totalPlantFood;
        float totalMeat;
        float averagePlantFood;
        float averageMeat;
        int tilesWithWater;
        int totalTiles;
    };
    
    GlobalResourceStats getGlobalStats() const {
        GlobalResourceStats stats{};
        
        for (const auto& [coord, resources] : tileResources) {
            stats.totalPlantFood += resources.plantFood;
            stats.totalMeat += resources.meat;
            if (resources.hasWater()) {
                stats.tilesWithWater++;
            }
            stats.totalTiles++;
        }
        
        if (stats.totalTiles > 0) {
            stats.averagePlantFood = stats.totalPlantFood / stats.totalTiles;
            stats.averageMeat = stats.totalMeat / stats.totalTiles;
        }
        
        return stats;
    }
};

#endif // RESOURCE_SYSTEM_HPP