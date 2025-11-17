#include "PerlinNoise.hpp"
#include "mathUtils.hpp"

PerlinNoise::PerlinNoise(int seed)
    : m_seed(seed) {}

void PerlinNoise::setSeed(int seed) {
    m_seed = seed;
}

double PerlinNoise::seededRandom(int x, int y) const {
    double n = std::sin(x * 12.9898 + y * 78.233 + m_seed * 43758.5453) * 43758.5453;
    return n - std::floor(n);
}

double PerlinNoise::smoothstep(double t) const {
    return t * t * (3.0 - 2.0 * t);
}

double PerlinNoise::noise(double x, double y) const {
    // Coordonnées entières
    int xi = static_cast<int>(std::floor(x));
    int yi = static_cast<int>(std::floor(y));
    
    // Partie fractionnaire
    double xf = x - xi;
    double yf = y - yi;
    
    // Lissage des coordonnées
    double u = smoothstep(xf);
    double v = smoothstep(yf);
    
    // Valeurs aléatoires aux 4 coins
    double aa = seededRandom(xi,     yi);
    double ab = seededRandom(xi,     yi + 1);
    double ba = seededRandom(xi + 1, yi);
    double bb = seededRandom(xi + 1, yi + 1);
    
    // Interpolation bilinéaire
    double x1 = MathUtils::lerp(aa, ba, u);
    double x2 = MathUtils::lerp(ab, bb, u);
    
    return MathUtils::lerp(x1, x2, v);
}

double PerlinNoise::fractalNoise(double x, double y, int octaves, 
                                 double persistence, double lacunarity) const
{
    double total = 0.0;
    double frequency = 1.0;
    double amplitude = 1.0;
    double maxValue = 0.0;
    
    for (int i = 0; i < octaves; ++i) {
        // Ajouter cette octave
        total += noise(x * frequency, y * frequency) * amplitude;
        
        // Accumuler la valeur maximale pour normalisation
        maxValue += amplitude;
        
        // Préparer la prochaine octave
        amplitude *= persistence;  // Diminuer l'amplitude
        frequency *= lacunarity;   // Augmenter la fréquence
    }
    
    // Normaliser le résultat entre 0 et 1
    return total / maxValue;
}
