#include "Noise.hpp"

#include <random>
#include <numeric>
#include <algorithm>
#include <cmath>

#include "mathUtils.hpp"

/**
 * Perlin's improved fade
 * 6t^5 - 15t^4 + 10t^3
 */
static double fade(double t) { 
    return t*t*t*(t*(t*6 - 15) + 10); 
}

/**
 * Classic Perlin gradient function for 2D
 */
static double grad(int hash, double x, double y) {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : (h == 12 || h == 14 ? x : 0.0);
    return (((h & 1) ? -u : u) + ((h & 2) ? -v : v));
}

void Noise::initPerm(unsigned int seed) {
    this->seed = seed;

    perm.resize(256);
    std::iota(perm.begin(), perm.end(), 0);

    std::mt19937 rng(seed);
    std::shuffle(perm.begin(), perm.end(), rng);

    // duplicate the permutation to avoid overflow in index
    perm.insert(perm.end(), perm.begin(), perm.end());
}

double Noise::noise(double x, double y) const {
    int xi = (int)std::floor(x) & 255;
    int yi = (int)std::floor(y) & 255;

    double xf = x - std::floor(x);
    double yf = y - std::floor(y);

    double u = fade(xf);
    double v = fade(yf);

    int A = perm[xi] + yi;
    int B = perm[xi + 1] + yi;

    // Hash coordinates of the 4 cube corners
    int aa = perm[perm[xi] + yi];
    int ab = perm[perm[xi] + yi + 1];
    int ba = perm[perm[xi + 1] + yi];
    int bb = perm[perm[xi + 1] + yi + 1];

    // Add blended results from corners
    double x1 = MathUtils::lerp(grad(aa, xf, yf), grad(ba, xf - 1.0, yf), u);
    double x2 = MathUtils::lerp(grad(ab, xf, yf - 1.0), grad(bb, xf - 1.0, yf - 1.0), u);
    double result = MathUtils::lerp(x1, x2, v);

    // result normally in range approx [-1,1]
    return result;
}

double Noise::fractalNoise(double x, double y, int octaves, double persistence, double lacunarity) const {
    double sum = 0.0;
    double amplitude = 1.0;
    double frequency = 1.0;
    double maxAmp = 0.0;

    for (int i = 0; i < octaves; ++i) {
        sum += noise(x * frequency, y * frequency) * amplitude;
        maxAmp += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    return 0.5 * (sum / (maxAmp + 1e-12) + 1.0);
}

double Noise::ridgedNoise(double x, double y, int octaves, double lacunarity, double gain) const {
    // Basic ridged transformation from FBM
    double fn = fractalNoise(x, y, octaves, lacunarity, gain); // [0, 1]
    double v = 2 * fn -1; // [-1,1]
    double r = 1.0 - std::fabs(v); // [0,1]
    return r; // higher values = ridges
}

unsigned int Noise::hash(unsigned int x) {
    x = (x ^ 61) ^ (x >> 16);
    x = x + (x << 3);
    x = x ^ (x >> 4);
    x = x * 0x27d4eb2d;
    x = x ^ (x >> 15);
    return x;
}

float Noise::valueNoise(int q, int r) {
    uint32_t h = hash(q * 374761393u + r * 668265263u + this->seed);
    return (h & 0xFFFFFF) / float(0xFFFFFF); // entre 0 et 1
}