#pragma once

#ifndef HEXCOORD_HPP
#define HEXCOORD_HPP

#include <vector>

struct HexCoord {
    float x;
    float y;

    bool operator==(const HexCoord& other) const {
        return x == other.x && y == other.y;
    }
};

enum hexNeighbors {
    Est, Nord_Est, Nord_Ouest, Ouest, Sud_Ouest, Sud_Est
};

namespace std {
    template<>
    struct hash<HexCoord> {
        std::size_t operator()(const HexCoord& hc) const noexcept {
            std::size_t h1 = std::hash<float>()(hc.x);
            std::size_t h2 = std::hash<float>()(hc.y);
            return h1 ^ (h2 << 1); // combinaison simple
        }
    };
}

#endif // HEXCOORD_HPP