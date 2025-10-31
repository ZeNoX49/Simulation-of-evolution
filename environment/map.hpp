#pragma once

#include <SFML/Graphics.hpp>

void createHexmap();

namespace map {
    inline std::vector<sf::ConvexShape> hexmap;
}