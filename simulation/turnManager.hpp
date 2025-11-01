#ifndef TURN_MANAGER_HPP
#define TURN_MANAGER_HPP

#include "../creatures/creature.hpp"
#include "../creatures/scent.hpp"
#include "../systems/feedingSystem.hpp"
#include "../systems/combatSystem.hpp"
#include "../systems/reproductionSystem.hpp"
#include "../systems/metabolismSystem.hpp"
#include "../systems/resourceSystem.hpp"
#include <vector>
#include <algorithm>

/**
 * Gestionnaire d'un tour complet de simulation
 * Orchestre tous les systèmes dans le bon ordre
 */
class TurnManager {
private:
    ResourceSystem& resourceSystem;
    FeedingSystem feedingSystem;
    CombatSystem combatSystem;
    ReproductionSystem reproductionSystem;
    MetabolismSystem metabolismSystem;
    ScentManager& scentManager;
    
    int currentTurn;
    
public:
    TurnManager(
        ResourceSystem& resourceSystem,
        std::unordered_map<HexCoord, TileResources>& tileResources,
        ScentManager& scentManager
    )
        : resourceSystem(resourceSystem),
          feedingSystem(tileResources),
          combatSystem(),
          reproductionSystem(),
          metabolismSystem(),
          scentManager(scentManager),
          currentTurn(0)
    {}
    
    // Exécuter un tour complet
    void executeTurn(std::vector<Creature>& creatures) {
        currentTurn++;
        
        // 1. Croissance des ressources des tiles
        resourceSystem.update();
        
        // 2. Décroissance des pistes olfactives
        scentManager.update();
        
        // 3. Update des créatures (déplacement, âge)
        updateCreatures(creatures);
        
        // 4. Actions des créatures (selon priorités)
        processCreatureActions(creatures);
        
        // 5. Métabolisme et vieillissement
        metabolismSystem.updateAll(creatures);
        applyAging(creatures);
        
        // 6. Cleanup des morts et ajout de meat
        int deadCount = metabolismSystem.cleanupDeadCreatures(
            creatures, 
            resourceSystem.getTileResources()
        );
    }
    
    int getCurrentTurn() const {
        return currentTurn;
    }
    
    void resetTurn() {
        currentTurn = 0;
    }
    
private:
    // Mettre à jour toutes les créatures (âge, déplacement)
    void updateCreatures(std::vector<Creature>& creatures) {
        for (auto& creature : creatures) {
            creature.update();
        }
    }
    
    // Appliquer le vieillissement (mort à 1000 tours)
    void applyAging(std::vector<Creature>& creatures) {
        const int MAX_AGE = 1000;
        
        for (auto& creature : creatures) {
            if (creature.isAlive && creature.age >= MAX_AGE) {
                creature.die();
            }
        }
    }
    
    // Traiter les actions de toutes les créatures
    void processCreatureActions(std::vector<Creature>& creatures) {
        // Mélanger l'ordre pour éviter les biais
        std::vector<size_t> indices;
        for (size_t i = 0; i < creatures.size(); ++i) {
            indices.push_back(i);
        }
        Random::shuffle(indices);
        
        // Vecteur pour stocker les nouveaux nés
        std::vector<Creature> newborns;
        
        // Traiter chaque créature
        for (size_t idx : indices) {
            Creature& creature = creatures[idx];
            
            if (!creature.isAlive) continue;
            if (creature.isMoving) continue;  // En déplacement, ne peut pas agir
            
            // Déterminer la priorité
            CreatureNeeds::Priority priority = creature.getCurrentPriority();
            
            switch (priority) {
                case CreatureNeeds::Priority::Hunger:
                    handleHunger(creature, creatures);
                    break;
                    
                case CreatureNeeds::Priority::Thirst:
                    handleThirst(creature);
                    break;
                    
                case CreatureNeeds::Priority::Love:
                    handleReproduction(creature, creatures, newborns);
                    break;
                    
                case CreatureNeeds::Priority::None:
                    // Rien à faire
                    break;
            }
        }
        
        // Ajouter les nouveaux nés
        for (auto& newborn : newborns) {
            creatures.push_back(newborn);
        }
    }
    
    // Gérer la faim
    void handleHunger(Creature& creature, std::vector<Creature>& allCreatures) {
        // Si carnivore, d'abord chercher des proies
        if (creature.stats.isCarnivore()) {
            // Chercher des proies sur la tile actuelle
            auto prey = combatSystem.findPotentialPrey(
                creature, 
                allCreatures, 
                creature.position
            );
            
            if (!prey.empty()) {
                // Attaquer la première proie
                auto outcome = combatSystem.attemptHunt(creature, *prey[0]);
                
                if (outcome.result == CombatSystem::CombatResult::PredatorWins) {
                    // Succès ! Exécuter le résultat
                    combatSystem.executeCombatResult(
                        creature, 
                        *prey[0], 
                        outcome,
                        resourceSystem.getTileResources()
                    );
                    
                    // Laisser une piste de peur si la proie s'est échappée
                    if (outcome.preyEscaped) {
                        scentManager.createScent(
                            prey[0]->position, 
                            prey[0]->id, 
                            Scent::Type::Fear
                        );
                    }
                    
                    return;  // Action terminée
                }
            }
        }
        
        // Essayer de manger des ressources (plantFood ou meat)
        auto result = feedingSystem.tryFeed(creature);
        
        if (result.success) {
            // Nourriture trouvée et consommée
            return;
        }
        
        if (result.needsToMove) {
            // Se déplacer vers la nourriture
            initiateMovement(creature, result.targetTile);
        }
    }
    
    // Gérer la soif
    void handleThirst(Creature& creature) {
        auto result = feedingSystem.seekWater(creature);
        
        if (result.success) {
            // Eau trouvée et consommée
            return;
        }
        
        if (result.needsToMove) {
            // Se déplacer vers l'eau
            initiateMovement(creature, result.targetTile);
        }
    }
    
    // Gérer la reproduction
    void handleReproduction(
        Creature& creature, 
        std::vector<Creature>& allCreatures,
        std::vector<Creature>& newborns
    ) {
        // Laisser une piste de recherche d'accouplement
        scentManager.createScent(
            creature.position, 
            creature.id, 
            Scent::Type::Mating
        );
        
        // Chercher un partenaire
        HexCoord mateLocation = reproductionSystem.getDirectionToMate(
            creature, 
            allCreatures, 
            scentManager
        );
        
        // Si un partenaire est trouvé et proche
        if (mateLocation != creature.position) {
            int distance = creature.position.distance(mateLocation);
            
            if (distance == 0) {
                // Sur la même tile, tenter reproduction
                auto result = reproductionSystem.attemptReproduction(
                    creature, 
                    allCreatures, 
                    scentManager
                );
                
                if (result.success && result.offspring) {
                    newborns.push_back(*result.offspring);
                    delete result.offspring;
                }
            } else if (distance == 1) {
                // Adjacent, se déplacer vers le partenaire
                initiateMovement(creature, mateLocation);
            } else {
                // Trop loin, se rapprocher
                initiateMovement(creature, mateLocation);
            }
        }
    }
    
    // Initier un déplacement (3 tours de base)
    void initiateMovement(Creature& creature, const HexCoord& target) {
        if (creature.position.distance(target) != 1) {
            // Pas adjacent, choisir la direction qui rapproche
            std::vector<HexCoord> neighbors = creature.position.neighbors();
            
            HexCoord bestNeighbor = creature.position;
            int bestDistance = creature.position.distance(target);
            
            for (const auto& neighbor : neighbors) {
                int dist = neighbor.distance(target);
                if (dist < bestDistance) {
                    bestDistance = dist;
                    bestNeighbor = neighbor;
                }
            }
            
            if (bestNeighbor != creature.position) {
                creature.moveTo(bestNeighbor);
                adjustMovementTime(creature);
                
                // Laisser une piste
                scentManager.createScent(
                    creature.position, 
                    creature.id, 
                    Scent::Type::Movement
                );
            }
        } else {
            // Adjacent, déplacement direct
            creature.moveTo(target);
            adjustMovementTime(creature);
            
            // Laisser une piste
            scentManager.createScent(
                creature.position, 
                creature.id, 
                Scent::Type::Movement
            );
        }
    }
    
    // Ajuster le temps de déplacement selon la vitesse
    // Vitesse de base = 10 → 3 tours
    // Vitesse 20 → 1.5 tours
    // Vitesse 5 → 6 tours
    void adjustMovementTime(Creature& creature) {
        const float BASE_SPEED = 10.0f;
        const int BASE_MOVEMENT_TIME = 3;
        
        // Calculer le temps selon la vitesse
        float speedRatio = BASE_SPEED / creature.stats.speed;
        int movementTime = static_cast<int>(std::ceil(BASE_MOVEMENT_TIME * speedRatio));
        
        // Appliquer la pénalité de stealth
        float stealthPenalty = creature.stats.getMovementSpeedPenalty();
        movementTime = static_cast<int>(std::ceil(movementTime * stealthPenalty));
        
        // Minimum 1 tour
        movementTime = std::max(1, movementTime);
        
        // Appliquer
        creature.movementCooldown = movementTime;
    }
    
public:
    // Obtenir des statistiques du tour
    struct TurnStats {
        int turn;
        int aliveCreatures;
        int movingCreatures;
        int hungryCreatures;
        int thirstyCreatures;
        int matingCreatures;
        int newborns;
        int deaths;
        
        TurnStats()
            : turn(0), aliveCreatures(0), movingCreatures(0),
              hungryCreatures(0), thirstyCreatures(0), matingCreatures(0),
              newborns(0), deaths(0)
        {}
    };
    
    TurnStats getTurnStats(const std::vector<Creature>& creatures) const {
        TurnStats stats;
        stats.turn = currentTurn;
        
        for (const auto& creature : creatures) {
            if (creature.isAlive) {
                stats.aliveCreatures++;
                
                if (creature.isMoving) {
                    stats.movingCreatures++;
                }
                
                CreatureNeeds::Priority priority = creature.getCurrentPriority();
                switch (priority) {
                    case CreatureNeeds::Priority::Hunger:
                        stats.hungryCreatures++;
                        break;
                    case CreatureNeeds::Priority::Thirst:
                        stats.thirstyCreatures++;
                        break;
                    case CreatureNeeds::Priority::Love:
                        stats.matingCreatures++;
                        break;
                    default:
                        break;
                }
            }
        }
        
        return stats;
    }
};

#endif // TURN_MANAGER_HPP