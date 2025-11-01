#ifndef CREATURE_RENDERER_HPP
#define CREATURE_RENDERER_HPP

#include <SFML/Graphics.hpp>
#include "../creatures/creature.hpp"
#include "../environment/hexCoord.hpp"
#include <vector>

/**
 * Système de rendu pour les créatures
 */
class CreatureRenderer {
private:
    float hexRadius;
    sf::Vector2f offset;
    
public:
    CreatureRenderer(float hexRadius, sf::Vector2f offset = sf::Vector2f(0, 0))
        : hexRadius(hexRadius), offset(offset)
    {}
    
    // Convertir HexCoord vers position écran
    sf::Vector2f hexToScreen(const HexCoord& coord) const {
        // Conversion flat-top
        float x = hexRadius * std::sqrt(3.0f) * (coord.q + coord.r / 2.0f);
        float y = hexRadius * 3.0f / 2.0f * coord.r;
        
        return sf::Vector2f(offset.x + x, offset.y + y);
    }
    
    // Dessiner une créature
    void drawCreature(sf::RenderWindow& window, const Creature& creature) const {
        if (!creature.isAlive) return;
        
        sf::Vector2f pos = hexToScreen(creature.position);
        
        // Taille selon la stat size (5-20 pixels)
        float displaySize = creature.getDisplaySize() * hexRadius * 0.15f;
        displaySize = std::max(3.0f, std::min(displaySize, hexRadius * 0.4f));
        
        // Créer un cercle
        sf::CircleShape circle(displaySize);
        circle.setOrigin({displaySize, displaySize});
        circle.setPosition(pos);
        
        // Couleur selon diet
        circle.setFillColor(creature.getDisplayColor());
        
        // Contour noir
        circle.setOutlineThickness(1.0f);
        circle.setOutlineColor(sf::Color::Black);
        
        // Si en mouvement, rendre semi-transparent
        if (creature.isMoving) {
            sf::Color color = circle.getFillColor();
            color.a = 150;
            circle.setFillColor(color);
        }
        
        window.draw(circle);
    }
    
    // Dessiner toutes les créatures
    void drawAllCreatures(sf::RenderWindow& window, const std::vector<Creature>& creatures) const {
        for (const auto& creature : creatures) {
            drawCreature(window, creature);
        }
    }
    
    // Dessiner avec info (pour debug)
    void drawCreatureWithInfo(
        sf::RenderWindow& window, 
        const Creature& creature,
        sf::Font& font
    ) const {
        if (!creature.isAlive) return;
        
        // Dessiner la créature
        drawCreature(window, creature);
        
        sf::Vector2f pos = hexToScreen(creature.position);
        
        // Texte avec infos
        std::stringstream ss;
        ss << "ID:" << creature.id << "\n"
           << "Age:" << creature.age << "\n"
           << "H:" << static_cast<int>(creature.needs.hunger) << "\n"
           << "T:" << static_cast<int>(creature.needs.thirst);

        sf::Text text(font, ss.str(), 10);
        text.setFillColor(sf::Color::White);
        text.setOutlineColor(sf::Color::Black);
        text.setOutlineThickness(1);
        text.setPosition({pos.x + hexRadius * 0.3f, pos.y - hexRadius * 0.3f});
        
        window.draw(text);
    }
    
    // Dessiner les indicateurs de besoins (barres de vie)
    void drawNeedsIndicators(sf::RenderWindow& window, const Creature& creature) const {
        if (!creature.isAlive) return;
        
        sf::Vector2f pos = hexToScreen(creature.position);
        float barWidth = hexRadius * 0.4f;
        float barHeight = 3.0f;
        float barSpacing = 5.0f;
        
        // Position de départ (au-dessus de la créature)
        float startY = pos.y - hexRadius * 0.3f;
        
        // Barre de hunger (rouge)
        drawBar(window, 
            sf::Vector2f(pos.x - barWidth / 2, startY),
            barWidth, barHeight,
            creature.needs.hunger / 100.0f,
            sf::Color::Red
        );
        
        // Barre de thirst (bleue)
        drawBar(window,
            sf::Vector2f(pos.x - barWidth / 2, startY - barSpacing),
            barWidth, barHeight,
            creature.needs.thirst / 100.0f,
            sf::Color::Blue
        );
        
        // Barre de love (rose) si mature
        if (creature.isMature()) {
            drawBar(window,
                sf::Vector2f(pos.x - barWidth / 2, startY - barSpacing * 2),
                barWidth, barHeight,
                creature.needs.love / 100.0f,
                sf::Color::Magenta
            );
        }
    }
    
private:
    // Helper pour dessiner une barre
    void drawBar(
        sf::RenderWindow& window,
        sf::Vector2f position,
        float width,
        float height,
        float fillRatio,  // 0-1
        sf::Color color
    ) const {
        // Background (gris)
        sf::RectangleShape bg(sf::Vector2f(width, height));
        bg.setPosition(position);
        bg.setFillColor(sf::Color(50, 50, 50));
        bg.setOutlineColor(sf::Color::Black);
        bg.setOutlineThickness(0.5f);
        window.draw(bg);
        
        // Foreground (couleur)
        if (fillRatio > 0.01f) {
            sf::RectangleShape fg(sf::Vector2f(width * fillRatio, height));
            fg.setPosition(position);
            fg.setFillColor(color);
            window.draw(fg);
        }
    }
    
public:
    // Mettre à jour l'offset (si la caméra bouge)
    void setOffset(sf::Vector2f newOffset) {
        offset = newOffset;
    }
    
    void setHexRadius(float radius) {
        hexRadius = radius;
    }
};

#endif // CREATURE_RENDERER_HPP