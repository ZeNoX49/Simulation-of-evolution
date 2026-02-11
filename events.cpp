#include "events.hpp"
#include "configuration.hpp"
#include "environment/map.hpp"
#include "gameParam.hpp"
#include <chrono>

#include <fstream>
#include <numeric>
#include <cstdio>
#include <iostream>
#include <algorithm>

void writeData();
double roundToTwo(float value);
float median(std::vector<float> values);

Camera* g_cam = nullptr; // pointeur global vers la caméra

auto last_x = std::chrono::high_resolution_clock::now();
auto last_y = std::chrono::high_resolution_clock::now();

int nbRotation = 6;
float movAdd = 1.57f / static_cast<float>(nbRotation);
float waitingDuration = 0.2f;

void initializeMouse(GLFWwindow* window) {
    g_cam = &gameUtils::cam;
    glfwSetScrollCallback(window, scroll_callback);
}

void processEvents(GLFWwindow* window) {
    /* ----- quitter ----- */
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        writeData();
        glfwSetWindowShouldClose(window, true);
    }

    /* ----- Mouvement de caméra ----- */
    // durée
    auto current_x = std::chrono::high_resolution_clock::now();
    float duration_x = static_cast<std::chrono::duration<float>>(current_x - last_x).count();

    auto current_y = std::chrono::high_resolution_clock::now();
    float duration_y = static_cast<std::chrono::duration<float>>(current_y - last_y).count();

    // mouvement vertical
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        if (duration_x > waitingDuration) {
            gameUtils::cam.pitch -= movAdd;
            last_x = current_x;
            if(gameUtils::cam.pitch < 0.00f) {
                gameUtils::cam.pitch = 0.00f;
            }
        }
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        if (duration_x > waitingDuration) {
            gameUtils::cam.pitch += movAdd;
            last_x = current_x;
            if(gameUtils::cam.pitch > 1.57f) {
                gameUtils::cam.pitch = 1.57f;
            }
        }
    }

    // mouvement horizontal
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if(duration_y > waitingDuration) {
            gameUtils::cam.yaw -= movAdd;
            last_y = current_y;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if(duration_y > waitingDuration) {
            gameUtils::cam.yaw += movAdd;
            last_y = current_y;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (!g_cam) return;

    // yoffset > 0 : scroll vers le haut
    // yoffset < 0 : scroll vers le bas
    g_cam->focalLenth -= yoffset * 1.0f; // multiplier par un facteur pour la vitesse
    if (g_cam->focalLenth < 5.0f) g_cam->focalLenth = 5.0f;     // limite min
    if (g_cam->focalLenth > 135.0f) g_cam->focalLenth = 135.0f; // limite max
}

/* ------------------------------------------------------------------------ */

void writeData() {
    std::ofstream file("map_data.txt");

    if (!file) {
        std::cerr << "Erreur : impossible d'ouvrir le fichier !" << std::endl;
        return;
    }

    file << "grid_size = " << gameParam::map_size << "\n"
         << "map_seed = " << gameParam::map_seed << "\n"
         << "map_octaves = " << gameParam::map_octaves << "\n"
         << "map_persistence = " << gameParam::map_persistence << "\n"
         << "map_lacunarity = " << gameParam::map_lacunarity << "\n"
         << "map_frequency = " << gameParam::map_frequency << "\n\n";

    file << "water_threshold = " << gameParam::water_threshold << "\n\n";

    file << "min_temp = " << gameParam::min_temp << "\n"
         << "max_temp = " << gameParam::max_temp << "\n"
         << "max_precipitation = " << gameParam::max_precipitation << "\n\n";

    std::unordered_map<std::string, int> biomes;
    std::vector<float> height_values;
    std::vector<float> temperature_values;
    std::vector<float> precipitation_values;
    for (const auto& [hc, tile] : map::hexmap) {
        std::string biome_name = tile.biome.name;
        if(biomes.find(biome_name) == biomes.end()) {
            biomes[biome_name] = 0;
        }
        biomes[biome_name]++;

        height_values.push_back(tile.height);
        temperature_values.push_back(tile.temperature);
        precipitation_values.push_back(tile.precipitation);
    }

    for (const auto& [name, nb] : biomes) {
        file << name << " : " << nb << "\n";
    }

    file << "\nHauteur :\n";
    float height_minimum = *std::min_element(height_values.begin(), height_values.end());
    file << "\t- minimum = " << roundToTwo(height_minimum) << "\n";
    float height_maximum = *std::max_element(height_values.begin(), height_values.end());
    file << "\t- maximum = " << roundToTwo(height_maximum) << "\n";
    float height_moyenne = std::accumulate(height_values.begin(), height_values.end(), 0.0f) / height_values.size();
    file << "\t- moyenne = " << roundToTwo(height_moyenne) << "\n";
    float height_medianne = median(height_values);
    file << "\t- medianne = " << roundToTwo(height_medianne)<< "\n\n";

    file << "Température :\n";
    float temperature_minimum = *std::min_element(temperature_values.begin(), temperature_values.end());
    file << "\t- minimum = " << roundToTwo(temperature_minimum) << "\n";
    float temperature_maximum = *std::max_element(temperature_values.begin(), temperature_values.end());
    file << "\t- maximum = " << roundToTwo(temperature_maximum) << "\n";
    float temperature_moyenne = std::accumulate(temperature_values.begin(), temperature_values.end(), 0.0f) / temperature_values.size();
    file << "\t- moyenne = " << roundToTwo(temperature_moyenne) << "\n";
    float temperature_medianne = median(temperature_values);
    file << "\t- medianne = " << roundToTwo(temperature_medianne)<< "\n\n";

    file << "Précipitation :\n";
    float precipitation_minimum = *std::min_element(precipitation_values.begin(), precipitation_values.end());
    file << "\t- minimum = " << roundToTwo(precipitation_minimum) << "\n";
    float precipitation_maximum = *std::max_element(precipitation_values.begin(), precipitation_values.end());
    file << "\t- maximum = " << roundToTwo(precipitation_maximum) << "\n";
    float precipitation_moyenne = std::accumulate(precipitation_values.begin(), precipitation_values.end(), 0.0f) / precipitation_values.size();
    file << "\t- moyenne = " << roundToTwo(precipitation_moyenne) << "\n";
    float precipitation_medianne = median(precipitation_values);
    file << "\t- medianne = " << roundToTwo(precipitation_medianne);

    file.close();

    std::printf("donnees ecrites\n");
}

double roundToTwo(float value) {
    double factor = std::pow(10.0, 2);
    return std::round(value * factor) / factor;
}

float median(std::vector<float> values) {
    if (values.empty()) return 0.0;

    std::sort(values.begin(), values.end());
    size_t n = values.size();

    if (n % 2 == 1) {
        // nombre impair : élément du milieu
        return values[n / 2];
    } else {
        // nombre pair : moyenne des deux éléments du milieu
        return (values[n / 2 - 1] + values[n / 2]) / 2.0;
    }
    return 0.0f;
}