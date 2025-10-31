#ifndef REPRODUCTION_SYSTEM_HPP
#define REPRODUCTION_SYSTEM_HPP

#include "../creatures/creature.hpp"
#include "../creatures/scent.hpp"
#include <vector>
#include <algorithm>

/**
 * Système de reproduction
 * - Recherche de partenaire
 * - Vérification de compatibilité
 * - Création de descendant avec mutation
 */
class ReproductionSystem {
public:
    struct ReproductionResult {
        bool success;
        Creature* offspring;
        
        ReproductionResult() : success(false), offspring(nullptr) {}
    };
    
    // Tenter de trouver un partenaire et se reproduire
    ReproductionResult attemptReproduction(
        Creature& creature,
        std::vector<Creature>& allCreatures,
        ScentManager& scentManager
    ) {
        ReproductionResult result;
        
        // Vérifications de base
        if (!creature.canReproduce()) {
            return result;
        }
        
        // Chercher un partenaire compatible
        Creature* partner = findMate(creature, allCreatures, scentManager);
        
        if (!partner) {
            // Pas de partenaire trouvé
            return result;
        }
        
        // Vérifier que le partenaire peut aussi reproduire
        if (!partner->canReproduce()) {
            return result;
        }
        
        // Vérifier la proximité (doivent être sur la même tile ou adjacente)
        int distance = creature.position.distance(partner->position);
        if (distance > 1) {
            return result;
        }
        
        // Reproduction réussie !
        result.success = true;
        
        // Créer le descendant
        HexCoord birthPosition = creature.position;  // Naît sur la position d'un parent
        Creature offspring = Creature::reproduce(creature, *partner, birthPosition);
        
        // Coût énergétique pour les parents
        creature.needs.payReproductionCost();
        partner->needs.payReproductionCost();
        
        // Réduire le désir de reproduction
        creature.needs.reduceLove(80.0f);
        partner->needs.reduceLove(80.0f);
        
        // Laisser une piste "mating"
        scentManager.createScent(creature.position, creature.id, Scent::Type::Mating);
        
        // Retourner le descendant (sera ajouté à la liste par le caller)
        result.offspring = new Creature(offspring);
        
        return result;
    }
    
private:
    // Trouver un partenaire compatible
    Creature* findMate(
        Creature& creature,
        std::vector<Creature>& allCreatures,
        ScentManager& scentManager
    ) {
        std::vector<Creature*> potentialMates;
        
        // Recherche dans un rayon
        int searchRadius = 3;
        
        for (auto& other : allCreatures) {
            if (!other.isAlive) continue;
            if (other.id == creature.id) continue;
            if (!other.canReproduce()) continue;
            
            // Vérifier la distance
            int distance = creature.position.distance(other.position);
            if (distance > searchRadius) continue;
            
            // Vérifier la compatibilité
            if (areCompatible(creature, other)) {
                potentialMates.push_back(&other);
            }
        }
        
        // Si pas de partenaire trouvé, chercher via les pistes olfactives
        if (potentialMates.empty()) {
            potentialMates = findMatesViaScent(creature, allCreatures, scentManager);
        }
        
        if (potentialMates.empty()) {
            return nullptr;
        }
        
        // Choisir le partenaire le plus proche
        std::sort(potentialMates.begin(), potentialMates.end(),
            [&creature](Creature* a, Creature* b) {
                return creature.position.distance(a->position) < 
                       creature.position.distance(b->position);
            }
        );
        
        return potentialMates[0];
    }
    
    // Vérifier la compatibilité entre deux créatures
    bool areCompatible(const Creature& a, const Creature& b) {
        // Même "espèce" = stats similaires (surtout diet)
        
        // Diet doit être dans une range similaire (±40)
        int dietDiff = std::abs(a.stats.diet - b.stats.diet);
        if (dietDiff > 40) return false;
        
        // Taille doit être dans une range similaire (±30%)
        float sizeRatio = a.stats.size / b.stats.size;
        if (sizeRatio < 0.7f || sizeRatio > 1.43f) return false;
        
        return true;
    }
    
    // Trouver des partenaires via les pistes olfactives
    std::vector<Creature*> findMatesViaScent(
        Creature& creature,
        std::vector<Creature>& allCreatures,
        ScentManager& scentManager
    ) {
        std::vector<Creature*> mates;
        
        // Chercher des pistes de type "Mating"
        std::vector<Scent> matingScents = scentManager.getScentsOfType(
            creature.position,
            Scent::Type::Mating,
            5  // radius
        );
        
        for (const auto& scent : matingScents) {
            // Vérifier si la créature peut détecter cette piste
            if (!scent.canBeDetectedBy(creature.stats.perception)) {
                continue;
            }
            
            // Trouver la créature correspondante
            for (auto& other : allCreatures) {
                if (other.id == scent.creatureId && other.isAlive) {
                    if (areCompatible(creature, other)) {
                        mates.push_back(&other);
                    }
                    break;
                }
            }
        }
        
        return mates;
    }
    
public:
    // Obtenir la direction vers un partenaire potentiel
    HexCoord getDirectionToMate(
        const Creature& creature,
        std::vector<Creature>& allCreatures,
        ScentManager& scentManager
    ) {
        // Chercher un partenaire
        Creature* mate = findMate(const_cast<Creature&>(creature), allCreatures, scentManager);
        
        if (!mate) {
            return creature.position;
        }
        
        // Si le partenaire est adjacent, retourner sa position
        if (creature.position.distance(mate->position) == 1) {
            return mate->position;
        }
        
        // Sinon, se déplacer vers lui (choisir la direction qui rapproche)
        std::vector<HexCoord> neighbors = creature.position.neighbors();
        
        HexCoord bestNeighbor = creature.position;
        int bestDistance = creature.position.distance(mate->position);
        
        for (const auto& neighbor : neighbors) {
            int dist = neighbor.distance(mate->position);
            if (dist < bestDistance) {
                bestDistance = dist;
                bestNeighbor = neighbor;
            }
        }
        
        return bestNeighbor;
    }
    
    // Vérifier si une créature peut potentiellement trouver un partenaire
    bool hasPotentialMates(
        const Creature& creature,
        const std::vector<Creature>& allCreatures,
        int searchRadius = 5
    ) const {
        for (const auto& other : allCreatures) {
            if (!other.isAlive) continue;
            if (other.id == creature.id) continue;
            if (!other.canReproduce()) continue;
            
            int distance = creature.position.distance(other.position);
            if (distance > searchRadius) continue;
            
            if (areCompatible(creature, other)) {
                return true;
            }
        }
        
        return false;
    }
    
    // Statistiques de reproduction (pour tracking)
    struct ReproductionStats {
        int totalBirths;
        int totalMutations;
        float averageGenerationGap;
        
        ReproductionStats() 
            : totalBirths(0), totalMutations(0), averageGenerationGap(0.0f) 
        {}
    };
};

#endif // REPRODUCTION_SYSTEM_HPP