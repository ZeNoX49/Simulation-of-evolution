#ifndef CREATURE_STATS_HPP
#define CREATURE_STATS_HPP

#include <algorithm>
#include "../utils/random.hpp"

/**
 * Stats évolutives d'une créature
 * Ces stats peuvent muter à la reproduction
 */
struct CreatureStats {
    float size;                 // Taille (affecte puissance d'attaque, consommation)
    float speed;                // Vitesse (affecte évasion, déplacement)
    float reproductionRate;     // Taux de reproduction (temps entre reproductions)
    int diet;                   // -99 (herbivore) à +99 (carnivore)
    float stealth;              // Discrétion (0-100, réduit vitesse déplacement)
    float perception;           // Détection (0-100, coût énergétique)
    
    // Valeurs par défaut (créature "équilibrée")
    CreatureStats()
        : size(10.0f),
          speed(10.0f),
          reproductionRate(10.0f),
          diet(0),
          stealth(10.0f),
          perception(10.0f)
    {}
    
    // Constructeur avec valeurs
    CreatureStats(float size, float speed, float reproductionRate, 
                  int diet, float stealth, float perception)
        : size(size),
          speed(speed),
          reproductionRate(reproductionRate),
          diet(diet),
          stealth(stealth),
          perception(perception)
    {
        clampValues();
    }
    
    // Forcer les valeurs dans les limites valides
    void clampValues() {
        size = std::max(1.0f, std::min(100.0f, size));
        speed = std::max(1.0f, std::min(100.0f, speed));
        reproductionRate = std::max(1.0f, std::min(100.0f, reproductionRate));
        diet = std::max(-99, std::min(99, diet));
        stealth = std::max(0.0f, std::min(100.0f, stealth));
        perception = std::max(0.0f, std::min(100.0f, perception));
    }
    
    // Calculer le modificateur de régime (0 à 1)
    // 0 = herbivore pur, 0.5 = omnivore, 1 = carnivore pur
    float getDietModifier() const {
        return (diet + 99.0f) / 198.0f;
    }
    
    // Vérifier si la créature est herbivore (diet < 0)
    bool isHerbivore() const {
        return diet < 0;
    }
    
    // Vérifier si la créature est carnivore (diet > 0)
    bool isCarnivore() const {
        return diet > 0;
    }
    
    // Vérifier si la créature est omnivore (diet proche de 0)
    bool isOmnivore() const {
        return std::abs(diet) < 20;
    }
    
    // Calculer la puissance d'attaque
    float getAttackPower() const {
        return size * getDietModifier();
    }
    
    // Calculer la capacité de chasse (quelle taille de proie peut-on attaquer ?)
    // Carnivore pur = 2x sa taille
    // Omnivore = 1.5x sa taille
    // Herbivore = 0 (ne chasse pas)
    float getMaxPreySize() const {
        if (diet <= 0) return 0.0f;  // Herbivores ne chassent pas
        
        float dietFactor = getDietModifier();  // 0.5 à 1.0
        // Map: 0.5 → 1.0, 1.0 → 2.0
        float multiplier = 1.0f + dietFactor;
        
        return size * multiplier;
    }
    
    // Calculer le coût métabolique de base
    float getBaseMetabolicRate() const {
        // Plus grand = plus de consommation
        // Perception augmente le coût
        return size * 0.1f + perception * 0.05f;
    }
    
    // Calculer la pénalité de vitesse due au stealth
    // Plus de stealth = plus lent pour se déplacer entre tiles
    float getMovementSpeedPenalty() const {
        // Stealth 0 = pas de pénalité
        // Stealth 100 = 50% plus lent
        return 1.0f + (stealth / 100.0f) * 0.5f;
    }
    
    // Calculer le bonus de détection de scents
    float getScentDetectionBonus() const {
        // Plus de perception = meilleure détection des pistes
        return perception / 100.0f;
    }
    
    // Créer un descendant avec héritage des stats (moyenne des parents)
    static CreatureStats inherit(const CreatureStats& parent1, const CreatureStats& parent2) {
        CreatureStats offspring;
        
        offspring.size = (parent1.size + parent2.size) / 2.0f;
        offspring.speed = (parent1.speed + parent2.speed) / 2.0f;
        offspring.reproductionRate = (parent1.reproductionRate + parent2.reproductionRate) / 2.0f;
        offspring.diet = (parent1.diet + parent2.diet) / 2;
        offspring.stealth = (parent1.stealth + parent2.stealth) / 2.0f;
        offspring.perception = (parent1.perception + parent2.perception) / 2.0f;
        
        offspring.clampValues();
        return offspring;
    }
    
    // Muter une stat (±30%)
    enum class MutableStat {
        Size,
        Speed,
        ReproductionRate,
        Diet,
        Stealth,
        Perception
    };
    
    void mutate(MutableStat stat) {
        // Mutation : ±30%
        float mutationFactor = Random::randFloat(0.7f, 1.3f);
        
        switch (stat) {
            case MutableStat::Size:
                size *= mutationFactor;
                break;
            case MutableStat::Speed:
                speed *= mutationFactor;
                break;
            case MutableStat::ReproductionRate:
                reproductionRate *= mutationFactor;
                break;
            case MutableStat::Diet:
                // Pour diet (entier), ajouter/soustraire jusqu'à 30
                diet += Random::randInt(-30, 30);
                break;
            case MutableStat::Stealth:
                stealth *= mutationFactor;
                break;
            case MutableStat::Perception:
                perception *= mutationFactor;
                break;
        }
        
        clampValues();
    }
    
    // Appliquer une mutation aléatoire (1/3 de chance)
    bool tryMutate() {
        if (Random::randBool(1.0f / 3.0f)) {
            // Choisir une stat aléatoire
            int statIndex = Random::randInt(0, 5);
            mutate(static_cast<MutableStat>(statIndex));
            return true;
        }
        return false;
    }
    
    // Calculer une "fitness" globale (pour stats/tracking)
    float getFitness() const {
        // Moyenne simple de toutes les stats normalisées
        return (size + speed + reproductionRate + stealth + perception) / 5.0f;
    }
};

#endif // CREATURE_STATS_HPP