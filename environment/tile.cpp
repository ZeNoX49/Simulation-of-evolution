#include <cmath>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include <string>

#include "tile.hpp"
#include "../game.hpp"
#include "../util/perlinNoise.hpp"

PerlinNoise moistureNoise(game::climate_seed);
PerlinNoise temperatureNoise(game::climate_seed + 1000);
float baseTemperature = 30.0f;      // Température de base à l'équateur (°C)
float poleTemperature = -40.0f;     // Température aux pôles (°C)
float altitudeTempLapse = 6.5f;     // Diminution de température par unité d'altitude (°C)
float maxRainfall = 325.0f;         // Précipitations maximum (mm/an)

// Calculer la latitude normalisée (0 = équateur, 1 = pôle)
float getLatitude(float y) {
    // Centre = équateur (0), bords = pôles (1)
    float normalizedY = static_cast<float>(y) / game::map_size;
    return std::abs(normalizedY - 0.5f) * 2.0f;
}

// Calculer la température en °C
float compute_temperature(float x, float y, float height) {
    float latitude = getLatitude(y);
    
    // Température de base selon la latitude (gradient équateur -> pôles)
    float latitudeTemp = baseTemperature - (baseTemperature - poleTemperature) * latitude;
    
    // Réduction due à l'altitude
    float altitudeEffect = -height * altitudeTempLapse * 10.0f; // *10 pour scaling
    
    // Variation locale avec Perlin Noise (±5°C)
    float noise = temperatureNoise.normalized2D(
        x * 0.05f, 
        y * 0.05f, 
        3,      // octaves
        0.5f,   // persistence
        2.0f,   // lacunarity
        1.0f    // frequency
    );
    float localVariation = (noise - 0.5f) * 10.0f;
    
    // Effet saisonnier léger (optionnel, peut être modifié avec un paramètre temps)
    float seasonalVariation = 0.0f;
    
    return latitudeTemp + altitudeEffect + localVariation + seasonalVariation;
}

// Calculer les précipitations en mm/an (entre 0 et 325)
float compute_precipitation(float x, float y, float height, float temperature) {
    float latitude = getLatitude(y);
    
    // Bandes climatiques (valeurs relatives entre 0 et 1)
    float latitudeRainfall = 0.5f;
    
    if (latitude < 0.2f) {
        // Zone équatoriale : forte pluie
        latitudeRainfall = 0.9f + (1.0f - latitude / 0.2f) * 0.1f;
    } else if (latitude < 0.4f) {
        // Zone tropicale : pluie décroissante
        float t = (latitude - 0.2f) / 0.2f;
        latitudeRainfall = 0.9f - t * 0.4f;
    } else if (latitude < 0.6f) {
        // Zone subtropicale/désertique : faible pluie
        float t = (latitude - 0.4f) / 0.2f;
        latitudeRainfall = 0.5f - t * 0.4f;
    } else if (latitude < 0.8f) {
        // Zone tempérée : pluie modérée
        float t = (latitude - 0.6f) / 0.2f;
        latitudeRainfall = 0.1f + t * 0.5f;
    } else {
        // Zone polaire : faible pluie (neige)
        latitudeRainfall = 0.6f - (latitude - 0.8f) / 0.2f * 0.3f;
    }
    
    // Effet de l'altitude (plus haut = moins de pluie en général)
    float altitudeEffect = 1.0f - height * 0.3f;
    
    // Humidité locale avec Perlin Noise
    float moisture = moistureNoise.normalized2D(
        x * 0.03f,
        y * 0.03f,
        4,      // octaves
        0.5f,   // persistence
        2.0f,   // lacunarity
        1.0f    // frequency
    );
    
    // Les zones froides ont moins d'évaporation donc moins de pluie
    float temperatureEffect = 1.0f;
    if (temperature < 0.0f) {
        temperatureEffect = 0.3f + 0.7f * (temperature + 40.0f) / 40.0f;
        temperatureEffect = std::max(0.1f, temperatureEffect);
    }
    
    // Calcul final (normalisé entre 0 et 1)
    float normalizedRainfall = latitudeRainfall * altitudeEffect * moisture * temperatureEffect;
    
    // Mise à l'échelle entre 0 et maxRainfall (325 mm/an)
    float rainfall = normalizedRainfall * maxRainfall;
    
    return std::clamp(rainfall, 0.0f, maxRainfall);
}

Biome define_biome(float x, float y, float height) {
    float temperature = compute_temperature(x, y, height);
    float precipitation = compute_precipitation(x, y, height, temperature);
    
    if(temperature >= 20 && precipitation >= 300) {
        return Biome{"Forêt tropicale humide", sf::Color(14,226,120), temperature, precipitation};
    }

    if(temperature >= 20 && precipitation >= 50) {
        return Biome{"Savane", sf::Color(255,218,63), temperature, precipitation};
    }

    if(temperature >= 5 && precipitation >= 200) {
        return Biome{"Forêt pluviale tempérée", sf::Color(21,203,107), temperature, precipitation};
    }

    if(temperature >= 5 && precipitation >= 100) {
        return Biome{"Forêt décidue", sf::Color(18,179,92), temperature, precipitation};
    }

    if(temperature >= 5 && precipitation >= 25) {
        return Biome{"Prairie", sf::Color(234,252,52), temperature, precipitation};
    }

    if(temperature >= -5 && precipitation >= 50) {
        return Biome{"Taïga", sf::Color(22,154,83), temperature, precipitation};
    }

    if(temperature >= -5 && precipitation >= 0) {
        return Biome{"Désert", sf::Color(255,144,165), temperature, precipitation};
    }

    if(temperature >= -30 && precipitation >= 0) {
        return Biome{"Toundra", sf::Color(175,217,235), temperature, precipitation};
    }

    return Biome{"Polaire", sf::Color(216,227,235), temperature, precipitation};
}