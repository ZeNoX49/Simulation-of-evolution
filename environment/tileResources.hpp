#ifndef TILE_RESOURCES_HPP
#define TILE_RESOURCES_HPP

#include <algorithm>

/**
 * Ressources disponibles sur une tile
 * - plantFood : nourriture végétale (croît jusqu'à capacité max)
 * - meat : cadavres (s'accumule, pourrit avec le temps)
 * - water : eau (basé sur proximité rivières, infini près d'elles)
 */
struct TileResources {
    // Ressources actuelles
    float plantFood;          // Quantité actuelle de nourriture végétale
    float meat;               // Quantité actuelle de viande
    float water;              // Disponibilité en eau (0 = sec, INFINITY = rivière)
    
    // Paramètres de croissance (dépendent du biome)
    float plantCapacity;      // Capacité maximale de plantFood
    float plantGrowthRate;    // Taux de croissance par tour
    float meatDecayRate;      // Taux de pourriture de la viande par tour
    
    // Constructeur par défaut
    TileResources() 
        : plantFood(0.0f),
          meat(0.0f),
          water(0.0f),
          plantCapacity(100.0f),
          plantGrowthRate(5.0f),
          meatDecayRate(0.1f)  // 10% par tour
    {}
    
    // Constructeur avec paramètres de biome
    TileResources(float capacity, float growthRate, float waterAvailability)
        : plantFood(capacity * 0.5f),  // Commence à 50% de la capacité
          meat(0.0f),
          water(waterAvailability),
          plantCapacity(capacity),
          plantGrowthRate(growthRate),
          meatDecayRate(0.1f)
    {}
    
    // Faire croître la nourriture végétale (appelé chaque tour)
    void growPlantFood() {
        if (plantFood < plantCapacity) {
            plantFood += plantGrowthRate;
            plantFood = std::min(plantFood, plantCapacity);
        }
    }
    
    // Faire pourrir la viande (appelé chaque tour)
    void decayMeat() {
        if (meat > 0.0f) {
            meat *= (1.0f - meatDecayRate);
            if (meat < 0.01f) {  // Seuil minimal
                meat = 0.0f;
            }
        }
    }
    
    // Consommer de la nourriture végétale
    // Retourne la quantité réellement consommée
    float consumePlantFood(float amount) {
        float consumed = std::min(amount, plantFood);
        plantFood -= consumed;
        return consumed;
    }
    
    // Consommer de la viande
    // Retourne la quantité réellement consommée
    float consumeMeat(float amount) {
        float consumed = std::min(amount, meat);
        meat -= consumed;
        return consumed;
    }
    
    // Consommer de l'eau
    // Retourne la quantité réellement consommée
    float consumeWater(float amount) {
        if (water == INFINITY) {
            return amount;  // Eau illimitée
        }
        
        float consumed = std::min(amount, water);
        water -= consumed;
        return consumed;
    }
    
    // Ajouter de la viande (quand une créature meurt)
    void addMeat(float amount) {
        meat += amount;
    }
    
    // Régénérer l'eau (si près d'une source)
    void regenerateWater(float amount) {
        if (water != INFINITY) {
            water = std::min(water + amount, 100.0f);
        }
    }
    
    // Vérifier si la tile a de la nourriture disponible pour herbivores
    bool hasPlantFood() const {
        return plantFood > 0.01f;
    }
    
    // Vérifier si la tile a de la viande disponible
    bool hasMeat() const {
        return meat > 0.01f;
    }
    
    // Vérifier si la tile a de l'eau disponible
    bool hasWater() const {
        return water > 0.01f || water == INFINITY;
    }
    
    // Obtenir le pourcentage de remplissage de plantFood
    float getPlantFoodRatio() const {
        return plantCapacity > 0.0f ? plantFood / plantCapacity : 0.0f;
    }
    
    // Vérifier si la tile est complètement vide de nourriture
    bool isEmpty() const {
        return !hasPlantFood() && !hasMeat();
    }
    
    // Reset complet des ressources
    void reset() {
        plantFood = 0.0f;
        meat = 0.0f;
        water = 0.0f;
    }
    
    // Reset vers valeurs initiales du biome
    void resetToInitial() {
        plantFood = plantCapacity * 0.5f;
        meat = 0.0f;
        // water reste inchangé (dépend de la géographie)
    }
};

#endif // TILE_RESOURCES_HPP