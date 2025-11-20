// RiverCarver.hpp
// Classe RiverCarver : applique un carving (abaissement) le long de trajets de rivières
#ifndef RIVER_CARVER_HPP
#define RIVER_CARVER_HPP

#include <vector>
#include <utility>

class RiverCarver {
public:
    RiverCarver() = default;
    ~RiverCarver() = default;

    // heights : row-major w*h
    // rivers : vecteur de chemins, chaque chemin = vect<pair<int x,int y>>
    // carveDepth : quantité à soustraire au centre de la rivière (0..1 dans la même échelle que heights)
    void Carve(std::vector<double> &heights, int w, int h,
               const std::vector<std::vector<std::pair<int,int>>> &rivers,
               double carveDepth = 0.02, int widthHalf = 2, bool smoothAfter = true);

private:
    void Smooth(std::vector<double> &heights, int w, int h, int passes = 1);
};

#endif // RIVER_CARVER_HPP