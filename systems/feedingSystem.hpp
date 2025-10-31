#ifndef FEEDING_SYSTEM_HPP
#define FEEDING_SYSTEM_HPP

#include "../creatures/creature.hpp"
#include "../environment/tileResources.hpp"
#include "../utils/random.hpp"
#include <unordered_map>
#include <vector>

/**
 * Système de recherche et consommation de nourriture
 * Implémente la logique du document (herbivore/carnivore/omnivore)
 */
class FeedingSystem {
private:
    std::unordered_map<HexCoord, TileResources>& tileResources;
    
public:
    FeedingSystem(std::unordered_map<HexCoord, TileResources>& resources)
        : tileResources(resources)
    {}
    
    // Résultat d'une tentative d'alimentation
    struct FeedingResult {
        bool success;           // A-t-on trouvé et consommé de la nourriture ?
        bool needsToMove;       // Doit-on se déplacer vers une tile adjacente ?
        HexCoord targetTile;    // Tile cible si needsToMove
        
        FeedingResult() : success(false), needsToMove(false), targetTile() {}
    };
    
    // Tenter de se nourrir (logique principale)
    FeedingResult tryFeed(Creature& creature) {
        FeedingResult result;
        
        if (creature.stats.isCarnivore()) {
            result = feedCarnivore(creature);
        } else if (creature.stats.isHerbivore()) {
            result = feedHerbivore(creature);
        } else {
            // Omnivore : essayer les deux
            result = feedOmnivore(creature);
        }
        
        return result;
    }
    
private:
    // Alimentation pour carnivore (diet > 0)
    FeedingResult feedCarnivore(Creature& creature) {
        FeedingResult result;
        TileResources* resources = getResources(creature.position);
        if (!resources) return result;
        
        // 1. Chercher meat sur tile actuelle
        if (resources->hasMeat()) {
            float consumed = resources->consumeMeat(creature.stats.size * 0.5f);
            creature.eatMeat(consumed);
            result.success = true;
            return result;
        }
        
        // 2. Pas de meat → chercher prey sur tile (géré par CombatSystem)
        // (On retourne juste que rien n'a été trouvé ici)
        
        // 3. Décision : chercher meat ailleurs ou manger plantFood ?
        int roll = Random::roll(99);
        
        if (roll >= creature.stats.diet) {
            // Chercher meat sur tiles adjacentes
            result = findFoodInAdjacentTiles(creature, true);  // true = cherche meat
        } else {
            // Essayer plantFood si disponible
            if (resources->hasPlantFood()) {
                float consumed = resources->consumePlantFood(creature.stats.size * 0.5f);
                creature.eatPlantFood(consumed);
                result.success = true;
            } else {
                // Tile complètement vide → se déplacer
                result = findFoodInAdjacentTiles(creature, false);  // cherche n'importe quoi
            }
        }
        
        return result;
    }
    
    // Alimentation pour herbivore (diet < 0)
    FeedingResult feedHerbivore(Creature& creature) {
        FeedingResult result;
        TileResources* resources = getResources(creature.position);
        if (!resources) return result;
        
        // 1. Chercher plantFood sur tile actuelle
        if (resources->hasPlantFood()) {
            float consumed = resources->consumePlantFood(creature.stats.size * 0.5f);
            creature.eatPlantFood(consumed);
            result.success = true;
            return result;
        }
        
        // 2. Pas de plantFood → décision
        int roll = Random::roll(99);
        int herbivoreThreshold = -creature.stats.diet;  // Inverser pour herbivore
        
        if (roll >= herbivoreThreshold) {
            // Chercher plantFood sur tiles adjacentes
            result = findFoodInAdjacentTiles(creature, false);  // false = cherche plants
        } else {
            // Essayer meat si disponible (rare pour herbivore)
            if (resources->hasMeat()) {
                float consumed = resources->consumeMeat(creature.stats.size * 0.3f);
                creature.eatMeat(consumed);
                result.success = true;
            } else {
                // Tile complètement vide → se déplacer
                result = findFoodInAdjacentTiles(creature, false);
            }
        }
        
        return result;
    }
    
    // Alimentation pour omnivore
    FeedingResult feedOmnivore(Creature& creature) {
        FeedingResult result;
        TileResources* resources = getResources(creature.position);
        if (!resources) return result;
        
        // Omnivore : essayer d'abord la préférence, puis l'autre
        bool prefersMeat = creature.stats.diet > 0;
        
        if (prefersMeat) {
            // Préfère meat
            if (resources->hasMeat()) {
                float consumed = resources->consumeMeat(creature.stats.size * 0.5f);
                creature.eatMeat(consumed);
                result.success = true;
                return result;
            }
            // Sinon essayer plantFood
            if (resources->hasPlantFood()) {
                float consumed = resources->consumePlantFood(creature.stats.size * 0.5f);
                creature.eatPlantFood(consumed);
                result.success = true;
                return result;
            }
        } else {
            // Préfère plantFood
            if (resources->hasPlantFood()) {
                float consumed = resources->consumePlantFood(creature.stats.size * 0.5f);
                creature.eatPlantFood(consumed);
                result.success = true;
                return result;
            }
            // Sinon essayer meat
            if (resources->hasMeat()) {
                float consumed = resources->consumeMeat(creature.stats.size * 0.5f);
                creature.eatMeat(consumed);
                result.success = true;
                return result;
            }
        }
        
        // Rien trouvé → chercher ailleurs
        result = findFoodInAdjacentTiles(creature, prefersMeat);
        return result;
    }
    
    // Chercher de la nourriture sur tiles adjacentes
    FeedingResult findFoodInAdjacentTiles(const Creature& creature, bool preferMeat) {
        FeedingResult result;
        
        std::vector<HexCoord> neighbors = creature.position.neighbors();
        Random::shuffle(neighbors);  // Aléatoire pour éviter biais
        
        HexCoord bestTile = creature.position;
        float bestScore = 0.0f;
        
        for (const auto& neighbor : neighbors) {
            TileResources* resources = getResources(neighbor);
            if (!resources) continue;
            
            float score = 0.0f;
            
            if (preferMeat) {
                score = resources->meat;
                // Bonus si aussi de l'eau
                if (resources->hasWater()) score += 10.0f;
            } else {
                score = resources->plantFood;
                // Bonus si aussi de l'eau
                if (resources->hasWater()) score += 10.0f;
            }
            
            if (score > bestScore) {
                bestScore = score;
                bestTile = neighbor;
            }
        }
        
        // Si on a trouvé une tile avec de la nourriture
        if (bestScore > 0.0f) {
            result.needsToMove = true;
            result.targetTile = bestTile;
        }
        
        return result;
    }
    
    // Boire de l'eau
    bool tryDrink(Creature& creature) {
        TileResources* resources = getResources(creature.position);
        if (!resources || !resources->hasWater()) {
            return false;
        }
        
        float consumed = resources->consumeWater(creature.stats.size * 0.3f);
        creature.drinkWater(consumed);
        return true;
    }
    
    // Chercher de l'eau sur tiles adjacentes
    HexCoord findWaterNearby(const Creature& creature) {
        std::vector<HexCoord> neighbors = creature.position.neighbors();
        
        // Trier par quantité d'eau disponible
        std::sort(neighbors.begin(), neighbors.end(), [this](const HexCoord& a, const HexCoord& b) {
            TileResources* resA = getResources(a);
            TileResources* resB = getResources(b);
            
            if (!resA) return false;
            if (!resB) return true;
            
            return resA->water > resB->water;
        });
        
        // Retourner la meilleure (ou position actuelle si rien)
        if (!neighbors.empty()) {
            TileResources* bestRes = getResources(neighbors[0]);
            if (bestRes && bestRes->hasWater()) {
                return neighbors[0];
            }
        }
        
        return creature.position;
    }
    
    // Helper : obtenir les ressources d'une tile
    TileResources* getResources(const HexCoord& coord) {
        auto it = tileResources.find(coord);
        return (it != tileResources.end()) ? &(it->second) : nullptr;
    }
    
public:
    // Gérer la recherche d'eau
    FeedingResult seekWater(Creature& creature) {
        FeedingResult result;
        
        // Essayer de boire sur place
        if (tryDrink(creature)) {
            result.success = true;
            return result;
        }
        
        // Chercher de l'eau à proximité
        HexCoord waterTile = findWaterNearby(creature);
        if (waterTile != creature.position) {
            result.needsToMove = true;
            result.targetTile = waterTile;
        }
        
        return result;
    }
};

#endif // FEEDING_SYSTEM_HPP