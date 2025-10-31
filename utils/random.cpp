#include "random.hpp"

// Initialisation des variables statiques
std::mt19937 Random::generator;
bool Random::initialized = false;