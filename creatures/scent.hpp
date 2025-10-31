#ifndef SCENT_HPP
#define SCENT_HPP

#include "../environment/hexCoord.hpp"

/**
 * Piste olfactive laissée par une créature
 * Utilisée pour tracking par prédateurs et partenaires
 */
struct Scent {
    enum class Type {
        Movement,       // Créature en mouvement normal
        Fear,          // Créature qui fuit (plus fort)
        Mating         // Créature cherchant partenaire
    };
    
    HexCoord position;      // Position de la piste
    int creatureId;         // ID de la créature qui a laissé la piste
    Type type;              // Type de piste
    float intensity;        // Intensité (0-100, décroît avec le temps)
    int age;                // Âge en tours
    int maxAge;             // Âge maximum avant disparition
    
    // Constructeur
    Scent(const HexCoord& pos, int creatureId, Type type, float intensity = 100.0f, int maxAge = 5)
        : position(pos),
          creatureId(creatureId),
          type(type),
          intensity(intensity),
          age(0),
          maxAge(maxAge)
    {}
    
    // Vieillir la piste (appelé chaque tour)
    void decay() {
        age++;
        
        // L'intensité décroît avec l'âge
        float decayRate = 1.0f / maxAge;
        intensity -= intensity * decayRate;
        
        if (intensity < 1.0f) {
            intensity = 0.0f;
        }
    }
    
    // Vérifier si la piste est encore valide
    bool isValid() const {
        return age < maxAge && intensity > 0.0f;
    }
    
    // Vérifier si la piste est expirée
    bool isExpired() const {
        return !isValid();
    }
    
    // Obtenir l'intensité effective selon la perception du détecteur
    float getEffectiveIntensity(float perception) const {
        // Plus de perception = meilleure détection des pistes faibles
        float perceptionBonus = perception / 100.0f;
        
        // Intensité de base + bonus
        float effective = intensity * (1.0f + perceptionBonus * 0.5f);
        
        return std::min(100.0f, effective);
    }
    
    // Vérifier si une créature peut détecter cette piste
    bool canBeDetectedBy(float perception, float distance = 0.0f) const {
        if (!isValid()) return false;
        
        float effective = getEffectiveIntensity(perception);
        
        // Plus on est loin, plus c'est difficile à détecter
        effective -= distance * 10.0f;
        
        // Seuil de détection
        return effective > 20.0f;
    }
    
    // Comparer les pistes (pour tri par intensité)
    bool operator<(const Scent& other) const {
        return intensity < other.intensity;
    }
    
    bool operator>(const Scent& other) const {
        return intensity > other.intensity;
    }
    
    // Obtenir la couleur pour le rendu (debug/visualisation)
    sf::Color getColor() const {
        uint8_t alpha = static_cast<uint8_t>(intensity * 2.55f);
        
        switch (type) {
            case Type::Movement:
                return sf::Color(100, 100, 100, alpha);  // Gris
            case Type::Fear:
                return sf::Color(255, 0, 0, alpha);      // Rouge
            case Type::Mating:
                return sf::Color(255, 0, 255, alpha);    // Magenta
            default:
                return sf::Color(128, 128, 128, alpha);
        }
    }
};

#include <SFML/Graphics/Color.hpp>

// Manager de pistes olfactives
class ScentManager {
private:
    std::vector<Scent> scents;
    
public:
    // Ajouter une nouvelle piste
    void addScent(const Scent& scent) {
        scents.push_back(scent);
    }
    
    // Créer et ajouter une piste
    void createScent(const HexCoord& pos, int creatureId, Scent::Type type) {
        // Durée selon le type
        int maxAge = 5;
        float intensity = 100.0f;
        
        switch (type) {
            case Scent::Type::Fear:
                maxAge = 3;      // Peur = piste courte mais intense
                intensity = 120.0f;
                break;
            case Scent::Type::Mating:
                maxAge = 8;      // Accouplement = piste longue
                intensity = 150.0f;
                break;
            default:
                break;
        }
        
        scents.emplace_back(pos, creatureId, type, intensity, maxAge);
    }
    
    // Vieillir toutes les pistes et nettoyer les expirées
    void update() {
        // Vieillir
        for (auto& scent : scents) {
            scent.decay();
        }
        
        // Supprimer les expirées
        scents.erase(
            std::remove_if(scents.begin(), scents.end(),
                [](const Scent& s) { return s.isExpired(); }),
            scents.end()
        );
    }
    
    // Trouver les pistes détectables par une créature à une position
    std::vector<Scent> getDetectableScents(const HexCoord& position, float perception, int radius = 2) const {
        std::vector<Scent> detected;
        
        for (const auto& scent : scents) {
            int distance = position.distance(scent.position);
            
            if (distance <= radius && scent.canBeDetectedBy(perception, static_cast<float>(distance))) {
                detected.push_back(scent);
            }
        }
        
        // Trier par intensité (plus fort d'abord)
        std::sort(detected.begin(), detected.end(), std::greater<Scent>());
        
        return detected;
    }
    
    // Trouver les pistes d'une créature spécifique
    std::vector<Scent> getScentsFromCreature(int creatureId) const {
        std::vector<Scent> result;
        
        for (const auto& scent : scents) {
            if (scent.creatureId == creatureId) {
                result.push_back(scent);
            }
        }
        
        return result;
    }
    
    // Trouver les pistes d'un type spécifique dans un rayon
    std::vector<Scent> getScentsOfType(const HexCoord& position, Scent::Type type, int radius = 3) const {
        std::vector<Scent> result;
        
        for (const auto& scent : scents) {
            if (scent.type == type && position.distance(scent.position) <= radius) {
                result.push_back(scent);
            }
        }
        
        return result;
    }
    
    // Obtenir toutes les pistes (pour rendu)
    const std::vector<Scent>& getAllScents() const {
        return scents;
    }
    
    // Nettoyer toutes les pistes
    void clear() {
        scents.clear();
    }
    
    // Obtenir le nombre de pistes actives
    size_t getCount() const {
        return scents.size();
    }
};

#endif // SCENT_HPP