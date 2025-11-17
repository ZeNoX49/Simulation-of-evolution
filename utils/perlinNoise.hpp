#pragma once

#include <cmath>

class PerlinNoise
{
public:
    // Constructeur avec seed
    PerlinNoise(int seed = 0);
    
    // Définir une nouvelle seed
    void setSeed(int seed);
    
    // Générer un bruit Perlin simple à une position (x, y)
    double noise(double x, double y) const;
    
    // Générer un bruit fractal (multiple octaves)
    double fractalNoise(double x, double y, int octaves, double persistence, double lacunarity) const;
    
private:
    int m_seed;
    
    // Générateur de nombres pseudo-aléatoires avec seed
    double seededRandom(int x, int y) const;
    
    // Fonction de lissage (smoothstep)
    double smoothstep(double t) const;
};