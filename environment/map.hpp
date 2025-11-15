#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "HexCoord.hpp"
#include "Tile.hpp"
#include "../rendering/graphicUtils.hpp"

namespace map {
    extern std::unordered_map<HexCoord, Tile> hexmap;
    extern std::vector<ObjData> hexmap_drawable;
    // std::vector<sf::Text> text_drawable;
    // sf::Font font;
}

void createHexmap();