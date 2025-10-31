#include "perlinNoise.hpp"

// Fonction de fade (interpolation lisse)
double PerlinNoise::fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

// Interpolation linéaire
double PerlinNoise::lerp(double t, double a, double b) {
    return a + t * (b - a);
}

// Calcul du gradient
double PerlinNoise::grad(int hash, double x, double y, double z) {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

// Constructeur avec seed
PerlinNoise::PerlinNoise(unsigned int seed) {
    permutation.resize(512);
    
    // Remplir avec les valeurs 0-255
    std::vector<int> p(256);
    std::iota(p.begin(), p.end(), 0);
    
    // Mélanger avec la seed
    std::default_random_engine engine(seed);
    std::shuffle(p.begin(), p.end(), engine);
    
    // Dupliquer pour éviter les débordements
    for (int i = 0; i < 256; i++) {
        permutation[i] = p[i];
        permutation[256 + i] = p[i];
    }
}

// Bruit Perlin de base (1 octave)
double PerlinNoise::noise(double x, double y, double z) const {
    // Trouver le cube unitaire contenant le point
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;
    
    // Position relative dans le cube
    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);
    
    // Calculer les courbes de fade
    double u = fade(x);
    double v = fade(y);
    double w = fade(z);
    
    // Hash des 8 coins du cube
    int A = permutation[X] + Y;
    int AA = permutation[A] + Z;
    int AB = permutation[A + 1] + Z;
    int B = permutation[X + 1] + Y;
    int BA = permutation[B] + Z;
    int BB = permutation[B + 1] + Z;
    
    // Interpoler les résultats
    return lerp(w,
        lerp(v,
            lerp(u, grad(permutation[AA], x, y, z),
                   grad(permutation[BA], x - 1, y, z)),
            lerp(u, grad(permutation[AB], x, y - 1, z),
                   grad(permutation[BB], x - 1, y - 1, z))),
        lerp(v,
            lerp(u, grad(permutation[AA + 1], x, y, z - 1),
                   grad(permutation[BA + 1], x - 1, y, z - 1)),
            lerp(u, grad(permutation[AB + 1], x, y - 1, z - 1),
                   grad(permutation[BB + 1], x - 1, y - 1, z - 1))));
}

// Bruit avec octaves multiples (fractal)
double PerlinNoise::octaveNoise(double x, double y, double z, 
                  int octaves, double persistence, 
                  double lacunarity, double frequency) const {
    double total = 0.0;
    double amplitude = 1.0;
    double maxValue = 0.0;
    double freq = frequency;
    
    for (int i = 0; i < octaves; i++) {
        total += noise(x * freq, y * freq, z * freq) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        freq *= lacunarity;
    }
    
    return total / maxValue; // Normaliser entre -1 et 1
}

// Version 2D simplifiée
double PerlinNoise::noise2D(double x, double y, int octaves, 
               double persistence, double lacunarity, 
               double frequency) const {
    return octaveNoise(x, y, 0.0, octaves, persistence, lacunarity, frequency);
}

// Version 3D complète
double PerlinNoise::noise3D(double x, double y, double z, int octaves,
               double persistence, double lacunarity,
               double frequency) const {
    return octaveNoise(x, y, z, octaves, persistence, lacunarity, frequency);
}

// Obtenir une valeur normalisée entre 0 et 1
double PerlinNoise::normalized2D(double x, double y, int octaves,
                   double persistence, double lacunarity,
                   double frequency) const {
    return (noise2D(x, y, octaves, persistence, lacunarity, frequency) + 1.0) * 0.5;
}

double PerlinNoise::normalized3D(double x, double y, double z, int octaves,
                   double persistence, double lacunarity,
                   double frequency) const {
    return (noise3D(x, y, z, octaves, persistence, lacunarity, frequency) + 1.0) * 0.5;
}