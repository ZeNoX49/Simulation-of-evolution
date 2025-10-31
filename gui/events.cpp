#include "events.hpp"

void processEvents(sf::Window& window, tgui::Gui& gui) {
    while (const std::optional event = window.pollEvent()) {
        if(event->is<sf::Event::Closed>()) {
            window.close();
        }

        else if(auto* key = event->getIf<sf::Event::KeyPressed>()) {                
            if (key->code == sf::Keyboard::Key::Escape) {
                window.close();
            }
        }

        const sf::Event& guiEvent = *event; // ✅ On déballe l'option
        gui.handleEvent(guiEvent);
    }
}