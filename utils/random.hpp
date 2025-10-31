#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>
#include <ctime>

/**
 * Système centralisé de génération de nombres aléatoires
 * Utilise Mersenne Twister pour une bonne qualité
 */
class Random {
private:
    static std::mt19937 generator;
    static bool initialized;
    
public:
    // Initialiser avec une seed spécifique
    static void init(unsigned int seed) {
        generator.seed(seed);
        initialized = true;
    }
    
    // Initialiser avec le temps actuel
    static void init() {
        generator.seed(static_cast<unsigned int>(std::time(nullptr)));
        initialized = true;
    }
    
    // S'assurer que le générateur est initialisé
    static void ensureInitialized() {
        if (!initialized) {
            init();
        }
    }
    
    // Nombre entier dans [min, max] (inclusif)
    static int randInt(int min, int max) {
        ensureInitialized();
        std::uniform_int_distribution<int> dist(min, max);
        return dist(generator);
    }
    
    // Nombre flottant dans [min, max)
    static float randFloat(float min, float max) {
        ensureInitialized();
        std::uniform_real_distribution<float> dist(min, max);
        return dist(generator);
    }
    
    // Nombre flottant dans [0, 1)
    static float randFloat() {
        return randFloat(0.0f, 1.0f);
    }
    
    // Boolean avec probabilité p (0.0 à 1.0)
    static bool randBool(float probability = 0.5f) {
        return randFloat() < probability;
    }
    
    // Roll de dé à N faces (1 à N)
    static int roll(int sides) {
        return randInt(1, sides);
    }
    
    // Gaussienne (distribution normale)
    static float randGaussian(float mean = 0.0f, float stddev = 1.0f) {
        ensureInitialized();
        std::normal_distribution<float> dist(mean, stddev);
        return dist(generator);
    }
    
    // Choisir un élément aléatoire dans un vecteur
    template<typename T>
    static const T& choice(const std::vector<T>& vec) {
        return vec[randInt(0, static_cast<int>(vec.size()) - 1)];
    }
    
    template<typename T>
    static T& choice(std::vector<T>& vec) {
        return vec[randInt(0, static_cast<int>(vec.size()) - 1)];
    }
    
    // Mélanger un vecteur (shuffle)
    template<typename T>
    static void shuffle(std::vector<T>& vec) {
        ensureInitialized();
        std::shuffle(vec.begin(), vec.end(), generator);
    }
    
    // Tirage sans remise (sample)
    template<typename T>
    static std::vector<T> sample(const std::vector<T>& vec, size_t n) {
        if (n > vec.size()) n = vec.size();
        
        std::vector<T> copy = vec;
        shuffle(copy);
        
        return std::vector<T>(copy.begin(), copy.begin() + n);
    }
    
    // Obtenir le générateur directement (pour distributions custom)
    static std::mt19937& getGenerator() {
        ensureInitialized();
        return generator;
    }
};

#endif // RANDOM_HPP