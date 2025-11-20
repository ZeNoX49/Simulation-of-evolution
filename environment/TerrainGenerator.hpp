// TerrainGenerator.hpp
#ifndef TERRAIN_GENERATOR_HPP
#define TERRAIN_GENERATOR_HPP

#include <vector>
#include <utility>
#include "../utils/PerlinNoise.hpp"

class TerrainGenerator {
public:
    int width = 256;
    int height = 256;
    double scale = 200.0;
    int octaves = 5;
    double persistence = 0.5;
    double lacunarity = 2.0;
    int erosionIterations = 600;
    double erosionTalus = 0.01;
    double riverFlowThreshold = 1000.0;
    int minRiverLength = 10;
    double seaLevel = 0.30;

    std::vector<double> heights;
    std::vector<double> flow;

    TerrainGenerator(unsigned seed = 1337);
    ~TerrainGenerator() = default;

    void GenerateHeightmap();
    void ApplyErosion(int iterations);
    void ComputeFlowAccumulation();
    std::vector<std::vector<std::pair<int,int>>> ExtractRivers(double threshold);
    void CarveRivers(const std::vector<std::vector<std::pair<int,int>>> &rivers,
                     double depth = 0.02, int widthHalf = 2, bool smoothAfter = true);
    void SmoothHeights(int passes = 1);

    std::vector<std::vector<std::pair<int,int>>> GenerateAll();

private:
    PerlinNoise noise;
    unsigned seed;

    inline int idx(int x, int y) const { return y * width + x; }
    void Normalize(std::vector<double> &m);
};

#endif // TERRAIN_GENERATOR_HPP