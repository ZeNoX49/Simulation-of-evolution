// PerlinNoise.hpp
#ifndef PERLIN_NOISE_HPP
#define PERLIN_NOISE_HPP

#include <vector>

class PerlinNoise {
public:
    PerlinNoise(unsigned seed = 1337);
    ~PerlinNoise() = default;

    void InitPerm(unsigned seed);

    double Noise(double x, double y) const;
    double FractalNoise(double x, double y,
                        int octaves,
                        double persistence,
                        double lacunarity) const;
private:
    std::vector<int> perm; // 512 entries
};

#endif // PERLIN_NOISE_HPP