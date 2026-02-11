#pragma once

#include <vector>

class Noise {
public:
    Noise() {};
    ~Noise() = default;

    void initPerm(unsigned int seed);

    double noise(double x, double y) const;
    
    double fractalNoise(double x, double y, int octaves, double persistence, double lacunarity) const;

    /**
     * Simple ridged noise (useful for mountains)
     */
    double ridgedNoise(double x, double y, int octaves = 6, double lacunarity = 2.0, double gain = 0.5) const;
    
    float valueNoise(int q, int r);

private:
    unsigned int seed;

    std::vector<int> perm; // 512 entries

    unsigned int hash(unsigned int x);
};