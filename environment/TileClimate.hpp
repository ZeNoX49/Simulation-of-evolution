// TileClimate.hpp
// Classe TileClimate : calcule température, précipitations et biome par tile
#ifndef TILE_CLIMATE_HPP
#define TILE_CLIMATE_HPP

#include <vector>

class TileClimate {
public:
    enum class Biome { Ocean = 0, Desert, Grassland, Forest, Taiga, Tundra, Snow, Mountain };

    // paramètres (modifiable)
    double equatorTemp = 30.0;
    double poleTemp = -20.0;
    double lapseRateCperKm = 6.5;
    double oceanInfluenceScale = 200.0; // en cellules
    double coastalTempBoost = 8.0;

    TileClimate() = default;
    ~TileClimate() = default;

    // heights: row-major size w*h
    // outTemp/outPrecip/outBiome: resized to w*h
    void ComputeClimate(const std::vector<double> &heights, int w, int h,
                        std::vector<double> &outTemp,
                        std::vector<double> &outPrecip,
                        std::vector<Biome> &outBiome,
                        double seaLevel = 0.30) const;
};

#endif // TILE_CLIMATE_HPP