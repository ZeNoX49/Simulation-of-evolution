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

    if (ImGui::SliderInt("Dimension", &gameParam::map_size, 5, 99))
        createHexmap();

    if (ImGui::SliderInt("Seed", &gameParam::map_seed, 0, 250))
        createHexmap();
    if (ImGui::SliderInt("Octaves", &gameParam::map_octaves, 1, 12))
        createHexmap();
    if (ImGui::SliderFloat("Persistence", &gameParam::map_persistence, 0.f, 5.f, "%.1f"))
        createHexmap();
    if (ImGui::SliderFloat("Lacunarité", &gameParam::map_lacunarity, 0.f, 12.f, "%.1f"))
        createHexmap();
    if (ImGui::SliderFloat("Fréquence", &gameParam::map_frequency, 0.01f, 0.5f, "%.02f"))
        createHexmap();

    if (ImGui::SliderInt("Offset X", &gameParam::offsetX, 0, 200))
        createHexmap();
    if (ImGui::SliderInt("Offset Y", &gameParam::offsetY, 0, 200))
        createHexmap();

    ImGui::Spacing();
    if (ImGui::SliderFloat("Seuil de l'eau", &gameParam::water_threshold, 0.f, 1.f, "%.3f"))
        createHexmap();
    if (ImGui::SliderFloat("Seuil de rivière", &gameParam::flow_threshold, 0.f, 1.f, "%.3f"))
        createHexmap();
    if (ImGui::SliderInt("mult de rivière", &gameParam::flow_mult, 1, 2500))
        createHexmap();
    if (ImGui::SliderInt("Nombre de value noise", &gameParam::nbVN, 1, 10))
        createHexmap();

    ImGui::Spacing();
    if (ImGui::SliderFloat("Température minimale", &gameParam::min_temp, -100.0f, 100.0f))
        createHexmap();
    if (ImGui::SliderFloat("Température maximale", &gameParam::max_temp, -100.0f, 100.0f))
        createHexmap();
    if (ImGui::SliderFloat("Précipitation maximale", &gameParam::max_precipitation, 0.0f, 2500.0f))
        createHexmap();

    ImGui::End();
}

void debuggingParameter() {
    ImGui::SetNextWindowPos(ImVec2(30, 30), ImGuiCond_Once);
    ImGui::Begin("debugging de la carte");

    ImGui::Spacing();
    ImGui::Text("Couleur des tiles : ");
    static const char* items[] = { "Biome", "Hauteur", "Température", "Précipitation" };
    if (ImGui::Combo("##TileColor", &gameParam::tile_color, items, IM_ARRAYSIZE(items)))
        createHexmap();

    ImGui::Spacing();
    ImGui::Checkbox("montrer la hauteur de l'eau", &gameParam::showWaterLevel);

    ImGui::Spacing();
    ImGui::Checkbox("montrer la hauteur maximal", &gameParam::showMaxHeight);

    ImGui::End();
}
