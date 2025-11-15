#pragma once

#ifndef PERLIN_NOISE_HPP
#define PERLIN_NOISE_HPP

#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>

class PerlinNoise {
private:
    std::vector<int> permutation;
    
    // Fonction de fade (interpolation lisse)
    static double fade(double t);
    
    // Calcul du gradient
    static double grad(int hash, double x, double y, double z);
    
public:
    // Constructeur avec seed
    PerlinNoise(unsigned int seed = 0);
    
    // Bruit Perlin de base (1 octave)
    double noise(double x, double y, double z = 0.0) const;
    
    // Bruit avec octaves multiples (fractal)
    double octaveNoise(double x, double y, double z, 
                      int octaves, double persistence = 0.5, 
                      double lacunarity = 2.0, double frequency = 1.0) const;
    
    // Version 2D simplifiée
    double noise2D(double x, double y, int octaves = 1, 
                   double persistence = 0.5, double lacunarity = 2.0, 
                   double frequency = 1.0) const;
    
    // Obtenir une valeur normalisée entre 0 et 1
    double normalized2D(double x, double y, int octaves = 1,
                       double persistence = 0.5, double lacunarity = 2.0,
                       double frequency = 1.0) const;
};

#endif // PERLIN_NOISE_HPP