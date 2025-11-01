#ifndef CREATURE_HPP
#define CREATURE_HPP

#include "creatureStats.hpp"
#include "creatureNeeds.hpp"
#include "scent.hpp"
#include "../environment/hexCoord.hpp"

/**
 * Créature principale de la simulation
 * Combine stats, besoins, position et état
 */
class Creature {
private:
    static int nextId;  // Pour générer des IDs uniques
    
public:
    int id;                     // ID unique
    HexCoord position;          // Position actuelle
    CreatureStats stats;        // Stats évolutives
    CreatureNeeds needs;        // Besoins (hunger, thirst, love)
    
    int age;                    // Âge en tours
    int maturityAge;            // Âge de maturité sexuelle
    int generation;             // Génération (pour tracking)
    
    bool isMoving;              // En déplacement ? (ne peut pas agir)
    int movementCooldown;       // Tours restants avant d'arriver
    HexCoord targetPosition;    // Destination si en déplacement
    
    bool isAlive;               // Vivante ou morte
    
    // Constructeur
    Creature(const HexCoord& startPos, const CreatureStats& stats, int generation = 0)
        : id(nextId++),
          position(startPos),
          stats(stats),
          needs(),
          age(0),
          maturityAge(10),  // Mature à 10 tours
          generation(generation),
          isMoving(false),
          movementCooldown(0),
          targetPosition(startPos),
          isAlive(true)
    {}
    
    // Constructeur par défaut (stats aléatoires)
    Creature(const HexCoord& startPos)
        : Creature(startPos, CreatureStats(), 0)
    {}
    
    // Mise à jour de la créature (appelé chaque tour)
    void update() {
        if (!isAlive) return;
        
        age++;
        
        // Gérer le déplacement
        if (isMoving) {
            movementCooldown--;
            if (movementCooldown <= 0) {
                // Arrivé à destination
                position = targetPosition;
                isMoving = false;
            }
            return;  // Ne peut pas agir pendant le déplacement
        }
        
        // Augmenter les besoins
        float metabolicRate = stats.getBaseMetabolicRate();
        float activityLevel = 1.0f;  // Peut varier selon l'action
        needs.increase(metabolicRate, activityLevel);
        
        // Augmenter le désir de reproduction si mature
        if (isMature()) {
            needs.increaseLove(stats.reproductionRate * 0.1f);
        }
        
        // Vérifier la mort
        if (needs.shouldDie()) {
            die();
        }
    }
    
    // Initier un déplacement vers une tile adjacente
    bool moveTo(const HexCoord& target) {
        if (isMoving) return false;
        if (position.distance(target) != 1) return false;  // Doit être adjacent
        
        targetPosition = target;
        isMoving = true;
        
        // Temps de déplacement de base : 3 tours
        // Modifié par la vitesse : vitesse de base (10) = 3 tours
        // Ex: vitesse 20 = 1.5 tours, vitesse 5 = 6 tours
        const float BASE_SPEED = 10.0f;
        const int BASE_MOVEMENT_TIME = 3;
        
        float speedRatio = BASE_SPEED / stats.speed;
        int baseTime = static_cast<int>(std::ceil(BASE_MOVEMENT_TIME * speedRatio));
        
        // Appliquer la pénalité de stealth
        float penalty = stats.getMovementSpeedPenalty();
        movementCooldown = static_cast<int>(std::ceil(baseTime * penalty));
        
        // Minimum 1 tour
        movementCooldown = std::max(1, movementCooldown);
        
        return true;
    }
    
    // Vérifier si la créature est mature
    bool isMature() const {
        return age >= maturityAge;
    }
    
    // Vérifier si la créature peut reproduire
    bool canReproduce() const {
        return isMature() && isAlive && !isMoving;
    }
    
    // Obtenir la priorité actuelle
    CreatureNeeds::Priority getCurrentPriority() const {
        return needs.getCurrentPriority(isMature());
    }
    
    // Manger de la nourriture végétale
    void eatPlantFood(float amount) {
        needs.satisfyHunger(amount * 2.0f);  // PlantFood satisfait bien
    }
    
    // Manger de la viande
    void eatMeat(float amount) {
        needs.satisfyHunger(amount * 3.0f);  // Viande satisfait mieux
    }
    
    // Boire de l'eau
    void drinkWater(float amount) {
        needs.satisfyThirst(amount * 2.0f);
    }
    
    // Tuer la créature
    void die() {
        isAlive = false;
    }
    
    // Obtenir la quantité de viande que produit cette créature
    float getMeatYield() const {
        // Plus grande créature = plus de viande
        return stats.size * 2.0f;
    }
    
    // Vérifier si cette créature peut être mangée par un prédateur
    bool canBeEatenBy(const Creature& predator) const {
        if (!isAlive) return false;
        if (predator.stats.diet <= 0) return false;  // Herbivore ne mange pas
        
        float maxPreySize = predator.stats.getMaxPreySize();
        return stats.size <= maxPreySize;
    }
    
    // Créer un descendant (reproduction entre deux parents)
    static Creature reproduce(const Creature& parent1, const Creature& parent2, const HexCoord& position) {
        // Hériter des stats (moyenne)
        CreatureStats offspringStats = CreatureStats::inherit(parent1.stats, parent2.stats);
        
        // 1/3 de chance de mutation
        offspringStats.tryMutate();
        
        // Créer le descendant
        int newGeneration = std::max(parent1.generation, parent2.generation) + 1;
        Creature offspring(position, offspringStats, newGeneration);
        
        // Besoins de départ pour nouveau-né
        offspring.needs.resetForNewborn();
        
        // Les parents paient le coût
        // (géré par le système de reproduction)
        
        return offspring;
    }
    
    // Obtenir une couleur pour le rendu (selon diet)
    sf::Color getDisplayColor() const {
        if (!isAlive) return sf::Color(50, 50, 50);  // Gris foncé si mort
        
        // Gradient selon diet
        // Herbivore = vert, Omnivore = jaune, Carnivore = rouge
        if (stats.diet < -50) {
            return sf::Color(0, 255, 0);        // Vert (herbivore)
        } else if (stats.diet < -20) {
            return sf::Color(150, 255, 0);      // Vert-jaune
        } else if (stats.diet < 20) {
            return sf::Color(255, 255, 0);      // Jaune (omnivore)
        } else if (stats.diet < 50) {
            return sf::Color(255, 150, 0);      // Orange
        } else {
            return sf::Color(255, 0, 0);        // Rouge (carnivore)
        }
    }
    
    // Obtenir la taille d'affichage (pour scaling du sprite)
    float getDisplaySize() const {
        return stats.size / 10.0f;  // Normaliser
    }
    
    // Générer un ID unique
    static int generateId() {
        return nextId++;
    }
    
    // Reset du compteur d'ID (pour nouvelle simulation)
    static void resetIdCounter() {
        nextId = 0;
    }
};

// Initialisation du compteur d'ID statique
int Creature::nextId = 0;

#endif // CREATURE_HPP