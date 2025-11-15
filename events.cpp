#include "events.hpp"
#include "configuration.hpp"
#include "environment/map.hpp"
#include "game.hpp"
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
float duration = 0.2f;

void initializeMouse(GLFWwindow* window) {
    g_cam = &game::cam;
    glfwSetScrollCallback(window, scroll_callback);
}

void processEvents(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
        
    // // Events GUI
    // const sf::Event& guiEvent = *event;
    // gui.handleEvent(guiEvent);
    
    // Flèches directionnelles

    auto current_x = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed_x = current_x - last_x;
    float duration_x = elapsed_x.count() * 1.0f;

    auto current_y = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed_y = current_y - last_y;
    float duration_y = elapsed_y.count() * 1.0f;

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        if (duration_x > duration && game::cam.pitch > (0.00f + movAdd)) {
            game::cam.pitch -= movAdd;
            last_x = current_x;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        if (duration_x > duration && game::cam.pitch < (1.57f-movAdd)) {
            game::cam.pitch += movAdd;
            last_x = current_x;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if(duration_y > duration) {
            game::cam.yaw -= movAdd;
            last_y = current_y;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if(duration_y > duration) {
            game::cam.yaw += movAdd;
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
    // std::ofstream file("map_data.txt");

    // if (!file) {
    //     std::cerr << "Erreur : impossible d'ouvrir le fichier !" << std::endl;
    //     return;
    // }

    // file << "grid_size = " << game::map_size << "\n"
    //      << "seuil = " << game::water_threshold << "\n"
    //      << "seed = " << game::water_seed << "\n"
    //      << "octaves = " << game::water_octaves << "\n"
    //      << "persitence = " << game::water_persistence << "\n"
    //      << "lacunarité = " << game::water_lacunarity << "\n"
    //      << "fréquence = " << game::water_frequency << "\n\n";

    // std::vector<float> height_values;
    // std::vector<float> temperature_values;
    // std::vector<float> precipitation_values;
    // for (const auto& [hc, tile] : map::hexmap) {
    //     // file << "x = " << roundToTwo(hc.x) << " / " << roundToTwo(tile.x) << "\t\t"
    //     //      << "y = " << roundToTwo(hc.y) << " / " << roundToTwo(tile.y) << "\t\t"
    //     //      << "biome = " << tile.biome.name << " -> (" << tile.biome.color.r <<  ", " << tile.biome.color.g <<  ", " << tile.biome.color.b << ")\t\t"
    //     //      << "height = " << roundToTwo(tile.height) << "\t\t"
    //     //      << "temperature = " << roundToTwo(tile.temperature) << "\t\t"
    //     //      << "precipitation = " << roundToTwo(tile.precipitation) << "\t\t"
    //     //      << std::endl;
    //     height_values.push_back(tile.height);
    //     temperature_values.push_back(tile.temperature);
    //     precipitation_values.push_back(tile.precipitation);
    // }

    // file << "Hauteur :\n";
    // float height_minimum = *std::min_element(height_values.begin(), height_values.end());
    // file << "\t- minimum = " << roundToTwo(height_minimum) << "\n";
    // float height_maximum = *std::max_element(height_values.begin(), height_values.end());
    // file << "\t- maximum = " << roundToTwo(height_maximum) << "\n";
    // float height_moyenne = std::accumulate(height_values.begin(), height_values.end(), 0.0f) / height_values.size();
    // file << "\t- moyenne = " << roundToTwo(height_moyenne) << "\n";
    // float height_medianne = median(height_values);
    // file << "\t- medianne = " << roundToTwo(height_medianne)<< "\n\n";

    // file << "Température :\n";
    // float temperature_minimum = *std::min_element(temperature_values.begin(), temperature_values.end());
    // file << "\t- minimum = " << roundToTwo(temperature_minimum) << "\n";
    // float temperature_maximum = *std::max_element(temperature_values.begin(), temperature_values.end());
    // file << "\t- maximum = " << roundToTwo(temperature_maximum) << "\n";
    // float temperature_moyenne = std::accumulate(temperature_values.begin(), temperature_values.end(), 0.0f) / temperature_values.size();
    // file << "\t- moyenne = " << roundToTwo(temperature_moyenne) << "\n";
    // float temperature_medianne = median(temperature_values);
    // file << "\t- medianne = " << roundToTwo(temperature_medianne)<< "\n\n";

    // file << "Précipitation :\n";
    // float precipitation_minimum = *std::min_element(precipitation_values.begin(), precipitation_values.end());
    // file << "\t- minimum = " << roundToTwo(precipitation_minimum) << "\n";
    // float precipitation_maximum = *std::max_element(precipitation_values.begin(), precipitation_values.end());
    // file << "\t- maximum = " << roundToTwo(precipitation_maximum) << "\n";
    // float precipitation_moyenne = std::accumulate(precipitation_values.begin(), precipitation_values.end(), 0.0f) / precipitation_values.size();
    // file << "\t- moyenne = " << roundToTwo(precipitation_moyenne) << "\n";
    // float precipitation_medianne = median(precipitation_values);
    // file << "\t- medianne = " << roundToTwo(precipitation_medianne);

    // file.close();

    // std::printf("donnees ecrites\n");
}

double roundToTwo(float value) {
    double factor = std::pow(10.0, 2);
    return std::round(value * factor) / factor;
}

float median(std::vector<float> values) {
    // if (values.empty()) return 0.0;

    // std::sort(values.begin(), values.end());
    // size_t n = values.size();

    // if (n % 2 == 1) {
    //     // nombre impair : élément du milieu
    //     return values[n / 2];
    // } else {
    //     // nombre pair : moyenne des deux éléments du milieu
    //     return (values[n / 2 - 1] + values[n / 2]) / 2.0;
    // }
    return 0.0f;
}