#include <cmath>
#include <algorithm>
#include <string>

#include "Tile.hpp"
#include "map.hpp"
#include "../gameParam.hpp"
// #include "../utils/PerlinNoise.hpp"
#include "../utils/mathUtils.hpp"

float baseTemperature = 30.0f;      // Température de base à l'équateur (°C)
float poleTemperature = -40.0f;     // Température aux pôles (°C)
float altitudeTempLapse = 6.5f;     // Diminution de température par unité d'altitude (°C)
float maxRainfall = 325.0f;         // Précipitations maximum (mm/an)

void Tile::setBiomeAquatic() {
    this->isWater = true;
    // D'ABORD calculer la température
    float temperature = compute_temperature(0);
    this->temperature = MathUtils::clamp(temperature, poleTemperature, baseTemperature);
    
    // ENSUITE calculer la précipitation (qui utilise this->temperature)
    float precipitation = compute_precipitation(0);
    this->precipitation = MathUtils::clamp(precipitation, 0.0f, maxRainfall);
    
    this->biome = getBiome(BiomeType::Water);
}

void Tile::define_biome(int nbAquaticNeighbors) {
    // D'ABORD calculer la température
    float temperature = compute_temperature(nbAquaticNeighbors);
    this->temperature = MathUtils::clamp(temperature, poleTemperature, baseTemperature);
    
    // ENSUITE calculer la précipitation (qui utilise this->temperature)
    float precipitation = compute_precipitation(nbAquaticNeighbors);
    this->precipitation = MathUtils::clamp(precipitation, 0.0f, maxRainfall);

    /* ---- Système de Whitaker simplifié ---- */

    // Tropical (≥ 20°C)
    if (this->temperature >= 20.0f && this->precipitation >= 300.0f) {
        this->biome = getBiome(BiomeType::Tropical_Rainforest);
    }
    else if (this->temperature >= 20.0f && this->precipitation > 50.0f) {
        this->biome = getBiome(BiomeType::Tropical_Savanna);
    }
    else if (this->temperature >= 20.0f) {
        this->biome = getBiome(BiomeType::Desert);
    }
    
    // Temperate (5-20°C)
    else if (this->temperature >= 5.0f && this->precipitation > 200.0f) {
        this->biome = getBiome(BiomeType::Temperate_Rainforest);
    }
    else if (this->temperature >= 5.0f && this->precipitation >= 100.0f) {
        this->biome = getBiome(BiomeType::Temperate_Deciduous_Forest);
    }
    else if (this->temperature >= 5.0f && this->precipitation >= 25.0f) {
        this->biome = getBiome(BiomeType::Temperate_Grassland);
    }
    else if (this->temperature >= 5.0f) {
        this->biome = getBiome(BiomeType::Desert);
    }
    
    // Cold (-5 to 5°C)
    else if (this->temperature >= -5.0f && this->precipitation >= 50.0f) {
        this->biome = getBiome(BiomeType::Taiga);
    }
    else if (this->temperature >= -5.0f) {
        this->biome = getBiome(BiomeType::Desert);
    }
    
    // Very Cold (< -5°C)
    else if (this->temperature >= -30.0f) {
        this->biome = getBiome(BiomeType::Tundra);
    }
    
    // Polar (< -30°C)
    else {
        this->biome = getBiome(BiomeType::Polar);
    }
}

Tile* Tile::getNeighbors(hexNeighbors neighbors) {
    HexCoord coord;
    switch (neighbors) {
        case hexNeighbors::Est:
            coord.x = this->hexCoord.x + 1.0f;
            coord.y = this->hexCoord.y;
            break;
        case hexNeighbors::Nord_Est:
            coord.x = this->hexCoord.x + 0.5f;
            coord.y = this->hexCoord.y - 1.0f;
            break;
        case hexNeighbors::Nord_Ouest:
            coord.x = this->hexCoord.x + 0.5f;
            coord.y = this->hexCoord.y - 1.0f;
            break;
        case hexNeighbors::Ouest:
            coord.x = this->hexCoord.x - 1.0f;
            coord.y = this->hexCoord.y;
            break;
        case hexNeighbors::Sud_Ouest:
            coord.x = this->hexCoord.x + 0.5f;
            coord.y = this->hexCoord.y + 1.0f;
            break;
        case hexNeighbors::Sud_Est:
            coord.x = this->hexCoord.x + 0.5f;
            coord.y = this->hexCoord.y + 1.0f;
            break;
    }

    auto it = map::hexmap.find(coord);
    if (it != map::hexmap.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<Tile*> Tile::getAllNeighbors() {
    std::vector<Tile*> neighbors;
    for (int i = 0; i < 6; ++i) {
        if(Tile* tile = getNeighbors(static_cast<hexNeighbors>(i))) {
            neighbors.push_back(tile);
        }
    }
    return neighbors;
}

// Calculer la latitude normalisée (0 = équateur, 1 = pôle)
float Tile::getLatitude() {
    // Normaliser d'abord entre 0 et 1
    float normalized_y = this->hexCoord.y / static_cast<float>(gameParam::map_size);
    // Centre = équateur (0), bords = pôles (1)
    return std::abs(normalized_y - 0.5f) * 2.0f;
}

// Calculer la température en °C
float Tile::compute_temperature(int nbAquaticNeighbors) {
    // Normaliser les coordonnées
    float latitude = getLatitude();
    
    // Température de base selon la latitude (gradient équateur -> pôles)
    float latitudeTemp = baseTemperature - (baseTemperature - poleTemperature) * latitude;
    
    // Réduction due à l'altitude (RÉDUITE : seulement pour les très hautes altitudes)
    float altitudeEffect = -height * altitudeTempLapse * 5.0f;
    
    // Variation locale avec Perlin Noise (±5°C)
    float localVariation = ((float)rand() / (float)RAND_MAX - 0.5f) * 10.0f;
    
    // Effet modérateur de l'eau (océans modèrent les températures)
    // Plus de voisins aquatiques = température plus modérée
    float waterModeration = 0.0f;
    if (nbAquaticNeighbors > 0) {
        // Rapprocher la température vers 15°C (température océanique moyenne)
        float oceanicTemp = 15.0f;
        float currentTemp = latitudeTemp + altitudeEffect + localVariation;
        // Plus il y a de voisins aquatiques, plus l'effet est fort (max 30% de modération)
        float moderationStrength = (nbAquaticNeighbors / 6.0f) * 0.3f;
        waterModeration = (oceanicTemp - currentTemp) * moderationStrength;
    }
    
    return latitudeTemp + altitudeEffect + localVariation + waterModeration;
}

// Calculer les précipitations en mm/an (entre 0 et 325)
float Tile::compute_precipitation(int nbAquaticNeighbors) {
    float latitude = getLatitude();
    
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
    
    // Les zones froides ont moins d'évaporation donc moins de pluie
    float temperatureEffect = 1.0f;
    if (this->temperature < 0.0f) {
        temperatureEffect = 0.3f + 0.7f * (this->temperature + 40.0f) / 40.0f;
        temperatureEffect = std::max(0.1f, temperatureEffect);
    }
    
    // Effet de proximité de l'eau (plus de voisins aquatiques = plus d'humidité)
    // Les océans sont des sources majeures d'humidité pour les précipitations
    float waterProximityBonus = 1.0f;
    if (nbAquaticNeighbors > 0) {
        // Bonus d'humidité : +5% à +50% selon le nombre de voisins aquatiques
        waterProximityBonus = 1.0f + (nbAquaticNeighbors / 6.0f) * 0.5f;
    }
    
    // Calcul final (normalisé entre 0 et 1)
    float normalizedRainfall = latitudeRainfall * altitudeEffect * ((float)rand() / (float)RAND_MAX) * temperatureEffect * waterProximityBonus;
    
    // Mise à l'échelle entre 0 et maxRainfall (325 mm/an)
    float rainfall = normalizedRainfall * maxRainfall;
    
    return rainfall;
}