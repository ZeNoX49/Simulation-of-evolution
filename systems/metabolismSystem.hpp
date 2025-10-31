#ifndef METABOLISM_SYSTEM_HPP
#define METABOLISM_SYSTEM_HPP

#include "../creatures/creature.hpp"
#include <vector>

/**
 * Système métabolique
 * - Gère l'augmentation des besoins selon métabolisme
 * - Applique les coûts de perception et stealth
 * - Détecte les morts par faim/soif
 */
class MetabolismSystem {
public:
    // Mettre à jour le métabolisme d'une créature
    void updateCreature(Creature& creature) {
        if (!creature.isAlive) return;
        if (creature.isMoving) return;  // Pas de métabolisme pendant déplacement
        
        // Calculer le taux métabolique de base
        float baseMetabolicRate = creature.stats.getBaseMetabolicRate();
        
        // Facteur d'activité (peut varier selon l'action)
        float activityLevel = getActivityLevel(creature);
        
        // Coût de perception (énergie)
        float perceptionCost = creature.stats.perception * 0.05f;
        
        // Augmenter hunger et thirst
        creature.needs.hunger += baseMetabolicRate * activityLevel + perceptionCost;
        creature.needs.thirst += (baseMetabolicRate * activityLevel + perceptionCost) * 0.5f;
        
        // Clamp
        creature.needs.hunger = std::min(100.0f, creature.needs.hunger);
        creature.needs.thirst = std::min(100.0f, creature.needs.thirst);
        
        // Augmenter le désir de reproduction si mature
        if (creature.isMature()) {
            float loveIncrease = creature.stats.reproductionRate * 0.1f;
            creature.needs.increaseLove(loveIncrease);
        }
        
        // Vérifier la mort
        if (creature.needs.shouldDie()) {
            creature.die();
        }
    }
    
    // Mettre à jour toutes les créatures
    void updateAll(std::vector<Creature>& creatures) {
        for (auto& creature : creatures) {
            updateCreature(creature);
        }
    }
    
private:
    // Obtenir le niveau d'activité selon l'action en cours
    float getActivityLevel(const Creature& creature) {
        // Pour l'instant, niveau standard
        // Peut être modifié selon l'action (combat = plus élevé, repos = plus bas)
        
        CreatureNeeds::Priority priority = creature.getCurrentPriority();
        
        switch (priority) {
            case CreatureNeeds::Priority::Hunger:
            case CreatureNeeds::Priority::Thirst:
                return 1.2f;  // Chercher activement = plus d'énergie
                
            case CreatureNeeds::Priority::Love:
                return 1.0f;  // Normal
                
            case CreatureNeeds::Priority::None:
                return 0.8f;  // Repos = moins d'énergie
                
            default:
                return 1.0f;
        }
    }
    
public:
    // Obtenir les créatures qui vont bientôt mourir
    std::vector<Creature*> getCriticalCreatures(std::vector<Creature>& creatures) {
        std::vector<Creature*> critical;
        
        for (auto& creature : creatures) {
            if (creature.isAlive && creature.needs.isCritical()) {
                critical.push_back(&creature);
            }
        }
        
        return critical;
    }
    
    // Obtenir les créatures mortes
    std::vector<Creature*> getDeadCreatures(std::vector<Creature>& creatures) {
        std::vector<Creature*> dead;
        
        for (auto& creature : creatures) {
            if (!creature.isAlive) {
                dead.push_back(&creature);
            }
        }
        
        return dead;
    }
    
    // Nettoyer les créatures mortes et ajouter meat aux tiles
    int cleanupDeadCreatures(
        std::vector<Creature>& creatures,
        std::unordered_map<HexCoord, TileResources>& tileResources
    ) {
        int count = 0;
        
        // Parcourir à l'envers pour supprimer en sécurité
        for (auto it = creatures.begin(); it != creatures.end(); ) {
            if (!it->isAlive) {
                // Ajouter meat sur la tile
                auto resIt = tileResources.find(it->position);
                if (resIt != tileResources.end()) {
                    float meatYield = it->getMeatYield();
                    resIt->second.addMeat(meatYield);
                }
                
                // Supprimer la créature
                it = creatures.erase(it);
                count++;
            } else {
                ++it;
            }
        }
        
        return count;
    }
    
    // Statistiques métaboliques
    struct MetabolismStats {
        int aliveCount;
        int deadCount;
        int criticalCount;
        float averageHunger;
        float averageThirst;
        float averageHealth;
        
        MetabolismStats()
            : aliveCount(0), deadCount(0), criticalCount(0),
              averageHunger(0.0f), averageThirst(0.0f), averageHealth(0.0f)
        {}
    };
    
    MetabolismStats getStats(const std::vector<Creature>& creatures) const {
        MetabolismStats stats;
        
        float totalHunger = 0.0f;
        float totalThirst = 0.0f;
        float totalHealth = 0.0f;
        
        for (const auto& creature : creatures) {
            if (creature.isAlive) {
                stats.aliveCount++;
                totalHunger += creature.needs.hunger;
                totalThirst += creature.needs.thirst;
                totalHealth += creature.needs.getHealthScore();
                
                if (creature.needs.isCritical()) {
                    stats.criticalCount++;
                }
            } else {
                stats.deadCount++;
            }
        }
        
        if (stats.aliveCount > 0) {
            stats.averageHunger = totalHunger / stats.aliveCount;
            stats.averageThirst = totalThirst / stats.aliveCount;
            stats.averageHealth = totalHealth / stats.aliveCount;
        }
        
        return stats;
    }
    
    // Appliquer un effet de vieillissement (optionnel)
    void applyAging(Creature& creature, int maxAge = 200) {
        if (creature.age >= maxAge) {
            // Mort de vieillesse
            creature.die();
        } else if (creature.age > maxAge * 0.8f) {
            // Vieillissement : métabolisme ralentit, stats diminuent légèrement
            float agingFactor = 0.99f;
            creature.stats.speed *= agingFactor;
            creature.stats.perception *= agingFactor;
        }
    }
    
    // Vérifier et appliquer vieillissement à toutes les créatures
    void applyAgingToAll(std::vector<Creature>& creatures, int maxAge = 200) {
        for (auto& creature : creatures) {
            if (creature.isAlive) {
                applyAging(creature, maxAge);
            }
        }
    }
};

#endif // METABOLISM_SYSTEM_HPP