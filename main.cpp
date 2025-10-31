#include <SFML/Graphics.hpp>

#define _USE_MATH_DEFINES
#include <cmath>

#include "guiParameter.hpp"
#include "events.hpp"
#include "configuration.hpp"
#include "game.hpp"
#include "environnement/map.hpp"

int main() {
    auto window = sf::RenderWindow{ sf::VideoMode::getFullscreenModes().front(), "Simulation de l'évolution", sf::Style::None, sf::State::Fullscreen };
    window.setFramerateLimit(conf::max_framerate);

    tgui::Gui gui{window};
    createParameter(gui);
    createHexmap();
    
    while (window.isOpen()) {
        processEvents(window, gui);

        window.clear(sf::Color(25, 25, 25));
        
        for (auto& hex : map::hexmap) {
            window.draw(hex);
        }

        gui.draw();
        
        window.display();
    }

    return 0;
}