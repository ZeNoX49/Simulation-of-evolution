#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "HexCoord.hpp"
#include "Tile.hpp"
#include "../rendering/graphicUtils.hpp"
#include "../object/tileModel.hpp"
#include "TerrainGenerator.hpp"

namespace map {
    inline std::unordered_map<HexCoord, Tile> hexmap;
    inline std::vector<ObjData> hexmap_drawable;
}

void createHexmap(TerrainGenerator& terrainGen);
