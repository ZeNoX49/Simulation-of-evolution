#include <cmath>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include <string>

#include "tile.hpp"
#include "../game.hpp"
#include "../utils/perlinNoise.hpp"

PerlinNoise moistureNoise(game::climate_seed);
PerlinNoise temperatureNoise(game::climate_seed + 1000);
float baseTemperature = 30.0f;      // Température de base à l'équateur (°C)
float poleTemperature = -40.0f;     // Température aux pôles (°C)
float altitudeTempLapse = 6.5f;     // Diminution de température par unité d'altitude (°C)
float maxRainfall = 325.0f;         // Précipitations maximum (mm/an)

// Calculer la latitude normalisée (0 = équateur, 1 = pôle)
float getLatitude(float normalizedY) {
    // Centre = équateur (0), bords = pôles (1)
    return std::abs(normalizedY - 0.5f) * 2.0f;
}

// Calculer la température en °C
// gridX et gridY sont les coordonnées de GRILLE (0 à map_size), pas les pixels !
float compute_temperature(float gridX, float gridY, float height) {
    // Normaliser les coordonnées
    float normalizedY = gridY / static_cast<float>(game::map_size);
    float latitude = getLatitude(normalizedY);
    
    // Température de base selon la latitude (gradient équateur -> pôles)
    float latitudeTemp = baseTemperature - (baseTemperature - poleTemperature) * latitude;
    
    // Réduction due à l'altitude (RÉDUITE : seulement pour les très hautes altitudes)
    float altitudeEffect = -height * altitudeTempLapse * 5.0f;  // Réduit de 10.0f à 5.0f
    
    // Variation locale avec Perlin Noise (±5°C)
    // Utiliser les coordonnées de grille multipliées par une fréquence
    float noise = temperatureNoise.normalized2D(
        gridX * 0.5f,  // Fréquence pour la variation locale
        gridY * 0.5f, 
        3,      // octaves
        0.5f,   // persistence
        2.0f,   // lacunarity
        1.0f    // frequency
    );
    float localVariation = (noise - 0.5f) * 10.0f;
    
    return latitudeTemp + altitudeEffect + localVariation;
}

// Calculer les précipitations en mm/an (entre 0 et 325)
float compute_precipitation(float gridX, float gridY, float height, float temperature) {
    float normalizedY = gridY / static_cast<float>(game::map_size);
    float latitude = getLatitude(normalizedY);
    
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
        gridX * 0.3f,  // Fréquence pour les variations d'humidité
        gridY * 0.3f,
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

Biome define_biome(float gridX, float gridY, float height) {
    float temperature = compute_temperature(gridX, gridY, height);
    float precipitation = compute_precipitation(gridX, gridY, height, temperature);
    
    // Système de Whitaker simplifié (basé sur le code C#)
    
    // Tropical (≥ 20°C)
    if (temperature >= 20.0f && precipitation >= 300.0f) {
        return Biome{"Tropical Rainforest", sf::Color(14, 226, 120), temperature, precipitation};
    }
    else if (temperature >= 20.0f && precipitation > 50.0f) {
        return Biome{"Tropical Savanna", sf::Color(255, 218, 63), temperature, precipitation};
    }
    else if (temperature >= 20.0f) {
        return Biome{"Desert", sf::Color(237, 201, 175), temperature, precipitation};
    }
    
    // Temperate (5-20°C)
    else if (temperature >= 5.0f && precipitation > 200.0f) {
        return Biome{"Temperate Rainforest", sf::Color(21, 203, 107), temperature, precipitation};
    }
    else if (temperature >= 5.0f && precipitation >= 100.0f) {
        return Biome{"Temperate Deciduous Forest", sf::Color(18, 179, 92), temperature, precipitation};
    }
    else if (temperature >= 5.0f && precipitation >= 25.0f) {
        return Biome{"Temperate Grassland", sf::Color(234, 252, 52), temperature, precipitation};
    }
    else if (temperature >= 5.0f) {
        return Biome{"Desert", sf::Color(255, 144, 165), temperature, precipitation};
    }
    
    // Cold (-5 to 5°C)
    else if (temperature >= -5.0f && precipitation >= 50.0f) {
        return Biome{"Boreal Forest (Taiga)", sf::Color(22, 154, 83), temperature, precipitation};
    }
    else if (temperature >= -5.0f) {
        return Biome{"Desert", sf::Color(210, 180, 140), temperature, precipitation};
    }
    
    // Very Cold (< -5°C)
    else if (temperature >= -30.0f) {
        return Biome{"Tundra", sf::Color(175, 217, 235), temperature, precipitation};
    }
    
    // Polar (< -30°C)
    return Biome{"Polar", sf::Color(216, 227, 235), temperature, precipitation};
}