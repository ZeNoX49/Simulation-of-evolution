#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "configuration.hpp"
#include "game.hpp"
#include "gui/guiParameter.hpp"
#include "environment/map.hpp"
#include "simulation/simulation.hpp"
#include "rendering/creatureRenderer.hpp"
#include "utils/random.hpp"

int main() {
    // Initialiser le random
    Random::init();
    
    // Fenêtre
    auto window = sf::RenderWindow{ 
        sf::VideoMode::getFullscreenModes().front(), 
        "Simulation de l'évolution", 
        sf::Style::None, 
        sf::State::Fullscreen 
    };
    window.setFramerateLimit(conf::max_framerate);
    
    // GUI
    tgui::Gui gui{window};
    createParameter(gui);
    
    // Générer la carte initiale
    std::cout << "Génération de la carte..." << std::endl;
    createHexmap();
    std::cout << "Carte générée!" << std::endl;
    
    // Créer la simulation
    Simulation simulation(game::map_size);
    
    // Initialiser avec les données de la carte
    std::cout << "Initialisation de la simulation..." << std::endl;
    auto mapData = getSimulationData();
    simulation.initialize(
        mapData.heightMap,
        mapData.biomeMap,
        mapData.waterDistances,
        20,  // 20 herbivores
        10,  // 10 carnivores
        15   // 15 omnivores
    );
    std::cout << "Simulation initialisée avec " << (20+10+15) << " créatures!" << std::endl;
    
    // Créer le renderer pour les créatures
    const float totalHeightFactor = 1.0f + (game::map_size - 1) * 0.75f;
    const float h = conf::game_window_size.y / totalHeightFactor;
    const float radius = h / 2.0f;
    const float w = std::sqrt(3.f) * radius;
    const float totalWidth = game::map_size * w;
    const float offsetX = conf::offsetX + (conf::game_window_size.x - totalWidth) / 2.0f + radius * std::sqrt(3.f) / 2.0f;
    const float offsetY = conf::offsetY + radius;
    
    CreatureRenderer creatureRenderer(radius, sf::Vector2f(offsetX, offsetY));
    
    // Variables de contrôle
    bool showCreatureInfo = false;
    bool showNeedsIndicators = true;
    int turnsPerFrame = 1;  // Vitesse de simulation
    bool autoUpdate = true; // Update automatique
    
    // Horloge pour auto-update
    sf::Clock clock;
    float updateInterval = 0.1f;  // 10 FPS de simulation par défaut
    
    // Texte pour afficher les stats à l'écran
    sf::Font font;
    // Essayer de charger une font (optionnel)
    // if (font.loadFromFile("arial.ttf")) {
    //     fontLoaded = true;
    // }
    
    // Texte des stats
    sf::Text statsText(font, "", 14);
    statsText.setFillColor(sf::Color::White);
    statsText.setOutlineColor(sf::Color::Black);
    statsText.setOutlineThickness(1);
    statsText.setPosition({10, 10});
    
    std::cout << "\n=== SIMULATION DÉMARRÉE ===" << std::endl;
    std::cout << "Contrôles:" << std::endl;
    std::cout << "  ESPACE : Pause/Resume" << std::endl;
    std::cout << "  I : Afficher/Masquer infos créatures" << std::endl;
    std::cout << "  N : Afficher/Masquer indicateurs de besoins" << std::endl;
    std::cout << "  + : Accélérer simulation" << std::endl;
    std::cout << "  - : Ralentir simulation" << std::endl;
    std::cout << "  S : Stats dans console" << std::endl;
    std::cout << "  R : Régénérer carte" << std::endl;
    std::cout << "  A : Toggle auto-update" << std::endl;
    std::cout << "  ENTRÉE : Avancer d'un tour (si pause)" << std::endl;
    std::cout << "  ESC : Quitter" << std::endl;
    std::cout << "==============================\n" << std::endl;
    
    // Boucle principale
    while (window.isOpen()) {
        // Gestion des événements
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            
            // Events GUI
            const sf::Event& guiEvent = *event;
            gui.handleEvent(guiEvent);
            
            // Contrôles clavier
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                switch (keyPressed->code) {
                    case sf::Keyboard::Key::Escape:
                        window.close();
                        break;
                        
                    case sf::Keyboard::Key::Space:
                        simulation.togglePause();
                        std::cout << (simulation.isPaused() ? "⏸ PAUSE" : "▶ RESUME") << std::endl;
                        break;
                        
                    case sf::Keyboard::Key::I:
                        showCreatureInfo = !showCreatureInfo;
                        std::cout << "Infos créatures : " << (showCreatureInfo ? "ON" : "OFF") << std::endl;
                        break;
                        
                    case sf::Keyboard::Key::N:
                        showNeedsIndicators = !showNeedsIndicators;
                        std::cout << "Indicateurs besoins : " << (showNeedsIndicators ? "ON" : "OFF") << std::endl;
                        break;
                        
                    case sf::Keyboard::Key::Equal:  // +
                        turnsPerFrame = std::min(10, turnsPerFrame + 1);
                        std::cout << "⚡ Vitesse: " << turnsPerFrame << " tours/frame" << std::endl;
                        break;
                        
                    case sf::Keyboard::Key::Hyphen:  // -
                        turnsPerFrame = std::max(1, turnsPerFrame - 1);
                        std::cout << "🐌 Vitesse: " << turnsPerFrame << " tours/frame" << std::endl;
                        break;
                        
                    case sf::Keyboard::Key::S:
                        simulation.printStats();
                        break;
                        
                    case sf::Keyboard::Key::A:
                        autoUpdate = !autoUpdate;
                        std::cout << "Auto-update : " << (autoUpdate ? "ON" : "OFF") << std::endl;
                        break;
                        
                    case sf::Keyboard::Key::Enter:
                        if (simulation.isPaused()) {
                            simulation.runNTurns(1);
                            std::cout << "→ Tour " << simulation.getCurrentTurn() << std::endl;
                        }
                        break;
                        
                    case sf::Keyboard::Key::R:
                        // Régénérer la carte et réinitialiser
                        std::cout << "\n🔄 Régénération..." << std::endl;
                        createHexmap();
                        simulation.reset();
                        auto mapData = getSimulationData();
                        simulation.initialize(mapData.heightMap, mapData.biomeMap, 
                                            mapData.waterDistances, 20, 10, 15);
                        std::cout << "✓ Régénération terminée!\n" << std::endl;
                        break;
                }
            }
        }
        
        // Update simulation (si pas en pause et auto-update activé)
        if (!simulation.isPaused() && autoUpdate && clock.getElapsedTime().asSeconds() >= updateInterval) {
            simulation.runNTurns(turnsPerFrame);
            clock.restart();
        }
        
        // Mettre à jour le texte des stats
        auto stats = simulation.getStats();
        std::stringstream ss;
        ss << "Tour: " << stats.currentTurn << "\n";
        ss << "Population: " << stats.population.totalPopulation << "\n";
        ss << "  Herbivores: " << stats.population.herbivores << "\n";
        ss << "  Carnivores: " << stats.population.carnivores << "\n";
        ss << "  Omnivores: " << stats.population.omnivores << "\n";
        ss << "Génération max: " << stats.population.maxGeneration << "\n";
        ss << std::fixed << std::setprecision(1);
        ss << "Âge moyen: " << stats.population.averageAge << "\n";
        ss << "FPS: " << (1.0f / clock.getElapsedTime().asSeconds());
        
        statsText.setString(ss.str());
        
        // Rendu
        window.clear(sf::Color(25, 25, 25));
        
        // Dessiner la carte
        for (const auto& hex : map::hexmap) {
            window.draw(hex);
        }
        
        // Dessiner les indicateurs de besoins (sous les créatures)
        if (showNeedsIndicators) {
            for (const auto& creature : simulation.getCreatures()) {
                creatureRenderer.drawNeedsIndicators(window, creature);
            }
        }
        
        // Dessiner les créatures
        creatureRenderer.drawAllCreatures(window, simulation.getCreatures());
        
        // Dessiner les infos détaillées (si activé et font chargée)
        if (showCreatureInfo) {
            for (const auto& creature : simulation.getCreatures()) {
                creatureRenderer.drawCreatureWithInfo(window, creature, font);
            }
        }
        
        // Dessiner les stats à l'écran
        window.draw(statsText);
        
        // Dessiner GUI
        gui.draw();
        
        window.display();
    }
    
    std::cout << "\n=== SIMULATION TERMINÉE ===" << std::endl;
    simulation.printStats();
    
    return 0;
}