#include "guiParameter.hpp"
#include "../game.hpp"
#include "../environment/map.hpp"
#include <imgui.h>

void createParameter()
{
    leftSide();
    rightSide();
}

void leftSide()
{
    ImGui::SetNextWindowPos(ImVec2(30, 30), ImGuiCond_Once);
    ImGui::Begin("Paramètres généraux");

    ImGui::Text("Dimension : ");
    if (ImGui::SliderInt("##Dimension", &game::map_size, 5, 99))
        createHexmap();

    ImGui::Spacing();
    ImGui::Text("Seed climat : ");
    if (ImGui::SliderInt("##SeedClimat", &game::climate_seed, 0, 250))
        createHexmap();

    ImGui::Spacing();
    ImGui::Text("Couleur des tiles : ");
    static const char* items[] = { "Biome", "Hauteur", "Température", "Précipitation" };
    if (ImGui::Combo("##TileColor", &game::tile_color, items, IM_ARRAYSIZE(items)))
        createHexmap();

    ImGui::End();
}

void rightSide()
{
    ImGui::SetNextWindowPos(ImVec2(1600, 30), ImGuiCond_Once);
    ImGui::Begin("Carte");

    ImGui::Text("------ Aquatique ------");
    ImGui::Spacing();

    if (ImGui::SliderFloat("Seuil", &game::water_threshold, 0.f, 1.f, "%.3f"))
        createHexmap();

    if (ImGui::SliderInt("Seed Eau", &game::water_seed, 0, 250))
        createHexmap();

    if (ImGui::SliderInt("Octaves Eau", &game::water_octaves, 1, 12))
        createHexmap();

    if (ImGui::SliderFloat("Persistence", &game::water_persistence, 0.f, 5.f, "%.1f"))
        createHexmap();

    if (ImGui::SliderFloat("Lacunarité", &game::water_lacunarity, 0.f, 12.f, "%.1f"))
        createHexmap();

    if (ImGui::SliderFloat("Fréquence", &game::water_frequency, 0.01f, 0.5f, "%.02f"))
        createHexmap();

    ImGui::End();
}
