#ifndef CREATURE_NEEDS_HPP
#define CREATURE_NEEDS_HPP

#include <algorithm>

/**
 * Besoins d'une créature (hunger, thirst, love)
 * Ces besoins augmentent avec le temps et déterminent les priorités
 */
struct CreatureNeeds {
    float hunger;       // 0-100 (0 = rassasié, 100 = affamé)
    float thirst;       // 0-100 (0 = hydraté, 100 = assoiffé)
    float love;         // 0-100 (0 = pas d'envie, 100 = cherche partenaire)
    
    // Constructeur (commence avec des besoins modérés)
    CreatureNeeds()
        : hunger(30.0f),
          thirst(30.0f),
          love(0.0f)
    {}
    
    // Augmenter les besoins selon le métabolisme et l'activité
    void increase(float metabolicRate, float activityLevel) {
        // Hunger et thirst augmentent avec métabolisme et activité
        hunger += metabolicRate * activityLevel;
        thirst += metabolicRate * activityLevel * 0.5f;  // Thirst augmente moins vite
        
        // Clamp entre 0 et 100
        hunger = std::min(100.0f, hunger);
        thirst = std::min(100.0f, thirst);
    }
    
    // Augmenter le désir de reproduction (si mature)
    void increaseLove(float amount) {
        love += amount;
        love = std::min(100.0f, love);
    }
    
    // Satisfaire la faim
    void satisfyHunger(float amount) {
        hunger -= amount;
        hunger = std::max(0.0f, hunger);
    }
    
    // Satisfaire la soif
    void satisfyThirst(float amount) {
        thirst -= amount;
        thirst = std::max(0.0f, thirst);
    }
    
    // Réduire le désir de reproduction (après reproduction)
    void reduceLove(float amount) {
        love -= amount;
        love = std::max(0.0f, love);
    }
    
    // Déterminer la priorité actuelle
    enum class Priority {
        Hunger,
        Thirst,
        Love,
        None
    };
    
    Priority getCurrentPriority(bool isMature) const {
        // Si pas mature, pas de reproduction
        if (!isMature) {
            // Choisir entre hunger et thirst
            if (hunger > 50.0f && hunger >= thirst) {
                return Priority::Hunger;
            } else if (thirst > 50.0f) {
                return Priority::Thirst;
            }
            return Priority::None;
        }
        
        // Si mature, comparer les 3 besoins
        // Seuils critiques
        const float criticalThreshold = 70.0f;
        const float loveThreshold = 60.0f;
        
        // Besoins critiques (survie)
        if (hunger >= criticalThreshold || thirst >= criticalThreshold) {
            return (hunger >= thirst) ? Priority::Hunger : Priority::Thirst;
        }
        
        // Reproduction si le besoin est fort
        if (love >= loveThreshold) {
            return Priority::Love;
        }
        
        // Sinon, prioriser selon le besoin le plus élevé
        if (hunger >= thirst && hunger >= love && hunger > 40.0f) {
            return Priority::Hunger;
        } else if (thirst >= hunger && thirst >= love && thirst > 40.0f) {
            return Priority::Thirst;
        } else if (love > 40.0f) {
            return Priority::Love;
        }
        
        return Priority::None;
    }
    
    // Vérifier si la créature est en danger de mort
    bool isCritical() const {
        return hunger >= 95.0f || thirst >= 95.0f;
    }
    
    // Vérifier si la créature doit mourir de faim/soif
    bool shouldDie() const {
        return hunger >= 100.0f || thirst >= 100.0f;
    }
    
    // Obtenir un score de santé global (0-100, 100 = très bien)
    float getHealthScore() const {
        // Inverser les besoins pour avoir un score de santé
        float hungerScore = 100.0f - hunger;
        float thirstScore = 100.0f - thirst;
        
        // Moyenne pondérée (hunger et thirst sont critiques)
        return (hungerScore + thirstScore) / 2.0f;
    }
    
    // Reset des besoins (pour nouveau-né)
    void resetForNewborn() {
        hunger = 20.0f;
        thirst = 20.0f;
        love = 0.0f;
    }
    
    // Coût énergétique de la reproduction
    void payReproductionCost() {
        hunger += 30.0f;
        thirst += 20.0f;
        hunger = std::min(100.0f, hunger);
        thirst = std::min(100.0f, thirst);
    }
    
    // Obtenir une description textuelle de la priorité
    static const char* priorityToString(Priority priority) {
        switch (priority) {
            case Priority::Hunger: return "Hunger";
            case Priority::Thirst: return "Thirst";
            case Priority::Love:   return "Love";
            case Priority::None:   return "None";
            default:               return "Unknown";
        }
    }
};

#endif // CREATURE_NEEDS_HPP