// PerlinNoise.cpp
#include "PerlinNoise.hpp"
#include <random>
#include <numeric>
#include <algorithm>
#include <cmath>

static double fade(double t) { return t*t*t*(t*(t*6 - 15) + 10); }
static double lerp(double t, double a, double b) { return a + t*(b - a); }
static double grad(int hash, double x, double y, double z = 0.0) {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return (((h & 1) ? -u : u) + ((h & 2) ? -v : v));
}

PerlinNoise::PerlinNoise(unsigned seed) {
    InitPerm(seed);
}

void PerlinNoise::InitPerm(unsigned seed) {
    perm.resize(512);
    std::vector<int> base(256);
    std::iota(base.begin(), base.end(), 0);

    std::mt19937 rng(seed);
    std::shuffle(base.begin(), base.end(), rng);

    for (int i = 0; i < 256; ++i) {
        perm[i] = base[i];
        perm[i + 256] = base[i];
    }
}

double PerlinNoise::Noise(double x, double y) const {
    int X = (int)std::floor(x) & 255;
    int Y = (int)std::floor(y) & 255;

    double xf = x - std::floor(x);
    double yf = y - std::floor(y);

    double u = fade(xf);
    double v = fade(yf);

    int A = perm[X] + Y, B = perm[X + 1] + Y;

    return lerp(v,
                lerp(u,
                     grad(perm[A], xf, yf),
                     grad(perm[B], xf - 1, yf)),
                lerp(u,
                     grad(perm[A + 1], xf, yf - 1),
                     grad(perm[B + 1], xf - 1, yf - 1)));
}

double PerlinNoise::FractalNoise(double x, double y,
                                 int octaves,
                                 double persistence,
                                 double lacunarity) const {
    double amplitude = 1.0;
    double frequency = 1.0;
    double sum = 0.0;
    double maxAmp = 0.0;

    for (int i = 0; i < octaves; ++i) {
        sum += Noise(x * frequency, y * frequency) * amplitude;
        maxAmp += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    return sum / (maxAmp + 1e-12);
}
