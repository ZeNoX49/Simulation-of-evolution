#include "guiParameter.hpp"
#include "../gameParam.hpp"
#include "../environment/map.hpp"
#include <imgui.h>

void createParameter() {
    mapParameter();
    debuggingParameter();
}

void mapParameter() {
    ImGui::SetNextWindowPos(ImVec2(1600, 30), ImGuiCond_Once);
    ImGui::Begin("Paramètres de la carte");
    ImGui::Spacing();

    if (ImGui::SliderInt("Dimension", &gameParam::map_size, 5, 99, "%2.0f"))
        createHexmap(gameUtils::terGen);

    if (ImGui::SliderInt("Seed", &gameParam::map_seed, 0, 250)) {
        gameUtils::terGen = TerrainGenerator(gameParam::map_seed);
        createHexmap(gameUtils::terGen);
    }

    if (ImGui::SliderInt("Octaves", &gameParam::map_octaves, 1, 12))
        createHexmap(gameUtils::terGen);

    if (ImGui::SliderFloat("Persistence", &gameParam::map_persistence, 0.f, 5.f, "%.1f"))
        createHexmap(gameUtils::terGen);

    if (ImGui::SliderFloat("Lacunarité", &gameParam::map_lacunarity, 0.f, 12.f, "%.1f"))
        createHexmap(gameUtils::terGen);

    if (ImGui::SliderFloat("Fréquence", &gameParam::map_frequency, 0.01f, 0.5f, "%.02f"))
        createHexmap(gameUtils::terGen);

    ImGui::Spacing();
    if (ImGui::SliderFloat("Seuil de l'eau", &gameParam::water_threshold, 0.f, 1.f, "%.3f"))
        createHexmap(gameUtils::terGen);

    if (ImGui::SliderFloat("erosion iterations", &gameParam::erosion_iterations, 0.f, 1.f, "%.3f"))
        createHexmap(gameUtils::terGen);

    if (ImGui::SliderFloat("river flow threshold", &gameParam::riverFlowThreshold, 0.f, 1.f, "%.3f"))
        createHexmap(gameUtils::terGen);

    ImGui::End();
}

void debuggingParameter() {
    ImGui::SetNextWindowPos(ImVec2(30, 30), ImGuiCond_Once);
    ImGui::Begin("Paramètres de la carte");

    ImGui::Spacing();
    ImGui::Text("Couleur des tiles : ");
    static const char* items[] = { "Biome", "Hauteur", "Température", "Précipitation" };
    if (ImGui::Combo("##TileColor", &gameParam::tile_color, items, IM_ARRAYSIZE(items)))
        createHexmap(gameUtils::terGen);

    ImGui::Spacing();
    ImGui::Checkbox("montrer la hauteur de l'eau", &gameParam::showWaterLevel);

    ImGui::Spacing();
    ImGui::Checkbox("montrer la hauteur maximal", &gameParam::showMaxHeight);

    ImGui::End();
}
