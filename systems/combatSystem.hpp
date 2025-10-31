#ifndef COMBAT_SYSTEM_HPP
#define COMBAT_SYSTEM_HPP

#include "../creatures/creature.hpp"
#include "../utils/random.hpp"
#include <vector>
#include <algorithm>

/**
 * Système de combat et prédation
 * Implémente la logique complète :
 * - Détection (perception vs stealth)
 * - Évasion (speed)
 * - Combat (size + diet)
 */
class CombatSystem {
public:
    enum class AttackState {
        NoAction,       // Prédateur ne détecte pas
        Ambush,         // Proie ne détecte pas (x2 attack)
        Normal          // Les deux se détectent
    };
    
    enum class CombatResult {
        PredatorWins,
        PreyWins,
        NoAction
    };
    
    struct CombatOutcome {
        CombatResult result;
        AttackState attackState;
        bool preyEscaped;
        
        CombatOutcome() 
            : result(CombatResult::NoAction), 
              attackState(AttackState::NoAction), 
              preyEscaped(false) 
        {}
    };
    
    // Tenter une attaque complète
    CombatOutcome attemptHunt(Creature& predator, Creature& prey) {
        CombatOutcome outcome;
        
        // Vérifier si le prédateur peut chasser
        if (predator.stats.diet <= 0) {
            outcome.result = CombatResult::NoAction;
            return outcome;
        }
        
        // Vérifier si la proie est mangeable
        if (!prey.canBeEatenBy(predator)) {
            outcome.result = CombatResult::NoAction;
            return outcome;
        }
        
        // Phase 1 : Détection
        bool predatorDetectsPrey = detectCreature(predator, prey);
        
        if (!predatorDetectsPrey) {
            // Prédateur ne voit pas la proie
            outcome.result = CombatResult::NoAction;
            outcome.attackState = AttackState::NoAction;
            return outcome;
        }
        
        bool preyDetectsPredator = detectCreature(prey, predator);
        
        // Déterminer le type d'attaque
        if (!preyDetectsPredator) {
            outcome.attackState = AttackState::Ambush;
        } else {
            outcome.attackState = AttackState::Normal;
        }
        
        // Phase 2 : Évasion
        bool preyEscapes = attemptEvasion(prey, predator);
        
        if (preyEscapes) {
            outcome.result = CombatResult::NoAction;
            outcome.preyEscaped = true;
            return outcome;
        }
        
        // Phase 3 : Combat
        outcome.result = resolveCombat(predator, prey, outcome.attackState);
        
        return outcome;
    }
    
    // Trouver des proies potentielles sur une tile
    std::vector<Creature*> findPotentialPrey(
        const Creature& predator,
        std::vector<Creature>& allCreatures,
        const HexCoord& position
    ) {
        std::vector<Creature*> prey;
        
        for (auto& creature : allCreatures) {
            if (!creature.isAlive) continue;
            if (creature.id == predator.id) continue;
            if (creature.position != position) continue;
            
            if (creature.canBeEatenBy(predator)) {
                prey.push_back(&creature);
            }
        }
        
        return prey;
    }
    
private:
    // Détection : observer peut-il détecter target ?
    bool detectCreature(const Creature& observer, const Creature& target) {
        // Calcul de détection : perception vs stealth, modifié par taille
        
        float perceptionScore = observer.stats.perception;
        float stealthScore = target.stats.stealth;
        
        // Modificateur de taille : plus grand = plus facile à voir
        float sizeModifier = target.stats.size / observer.stats.size;
        
        // Si la cible est 2x plus grande, +50% de chance de détection
        // Si la cible est 2x plus petite, -25% de chance de détection
        float detectionChance = perceptionScore - (stealthScore * (2.0f - sizeModifier));
        
        // Normaliser entre 0 et 100
        detectionChance = std::max(10.0f, std::min(90.0f, detectionChance));
        
        // Roll
        float roll = Random::randFloat(0.0f, 100.0f);
        
        return roll < detectionChance;
    }
    
    // Évasion : la proie peut-elle s'échapper ?
    bool attemptEvasion(const Creature& prey, const Creature& predator) {
        // Calcul d'évasion basé sur la vitesse relative
        
        float preySpeed = prey.stats.speed;
        float predatorSpeed = predator.stats.speed;
        
        // Chance d'évasion = vitesse de la proie / vitesse totale
        float totalSpeed = preySpeed + predatorSpeed;
        float evasionChance = (preySpeed / totalSpeed) * 100.0f;
        
        // Bonus si la proie a détecté le prédateur (peut anticiper)
        bool preyDetectedPredator = detectCreature(prey, predator);
        if (preyDetectedPredator) {
            evasionChance += 10.0f;
        }
        
        // Clamp entre 10% et 80%
        evasionChance = std::max(10.0f, std::min(80.0f, evasionChance));
        
        // Roll
        float roll = Random::randFloat(0.0f, 100.0f);
        
        return roll < evasionChance;
    }
    
    // Combat : qui gagne ?
    CombatResult resolveCombat(
        const Creature& predator,
        const Creature& prey,
        AttackState attackState
    ) {
        // Calcul de puissance d'attaque
        float dietModifier = predator.stats.getDietModifier();
        float predatorAttackPower = predator.stats.size * dietModifier;
        
        // Bonus d'embuscade
        if (attackState == AttackState::Ambush) {
            predatorAttackPower *= 2.0f;
        }
        
        // Puissance de défense de la proie
        float preyDietModifier = prey.stats.getDietModifier();
        float preyDefensePower = prey.stats.size * preyDietModifier;
        
        // Total
        float totalPower = predatorAttackPower + preyDefensePower;
        
        // Roll
        float roll = Random::randFloat(0.0f, totalPower);
        
        if (roll > predatorAttackPower) {
            return CombatResult::PreyWins;
        } else {
            return CombatResult::PredatorWins;
        }
    }
    
public:
    // Exécuter le résultat du combat (tuer la proie, nourrir le prédateur)
    void executeCombatResult(
        Creature& predator,
        Creature& prey,
        const CombatOutcome& outcome,
        std::unordered_map<HexCoord, TileResources>& tileResources
    ) {
        if (outcome.result == CombatResult::PredatorWins) {
            // Prédateur gagne : manger la proie
            float meatYield = prey.getMeatYield();
            predator.eatMeat(meatYield);
            
            // Tuer la proie
            prey.die();
            
            // Pas de cadavre (mangé directement)
            // Ou optionnel : laisser un peu de meat sur la tile
            auto it = tileResources.find(prey.position);
            if (it != tileResources.end()) {
                it->second.addMeat(meatYield * 0.2f);  // 20% reste
            }
        }
        // Si PreyWins ou NoAction : rien ne se passe
    }
    
    // Obtenir des stats de combat (pour debug/affichage)
    struct CombatStats {
        float predatorAttackPower;
        float preyDefensePower;
        float detectionChance;
        float evasionChance;
    };
    
    CombatStats getCombatStats(const Creature& predator, const Creature& prey) const {
        CombatStats stats;
        
        stats.predatorAttackPower = predator.stats.getAttackPower();
        stats.preyDefensePower = prey.stats.size * prey.stats.getDietModifier();
        
        // Détection simplifiée
        float perceptionScore = predator.stats.perception;
        float stealthScore = prey.stats.stealth;
        float sizeModifier = prey.stats.size / predator.stats.size;
        stats.detectionChance = perceptionScore - (stealthScore * (2.0f - sizeModifier));
        stats.detectionChance = std::max(10.0f, std::min(90.0f, stats.detectionChance));
        
        // Évasion
        float totalSpeed = prey.stats.speed + predator.stats.speed;
        stats.evasionChance = (prey.stats.speed / totalSpeed) * 100.0f;
        stats.evasionChance = std::max(10.0f, std::min(80.0f, stats.evasionChance));
        
        return stats;
    }
};

#endif // COMBAT_SYSTEM_HPP