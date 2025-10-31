#include "guiParameter.hpp"
#include "game.hpp"
#include "environnement/map.hpp"
#include <sstream>
#include <iomanip>

// Fonction helper pour créer un label
auto createLabel(const std::string& text, float x, float y, unsigned int size = 20, 
                 tgui::Color color = tgui::Color::White, bool bold = false) {
    auto label = tgui::Label::create(text);
    label->setPosition(x, y);
    label->setTextSize(size);
    label->getRenderer()->setTextColor(color);
    if (bold) {
        label->getRenderer()->setTextStyle(tgui::TextStyle::Bold);
    }
    return label;
}

// Fonction helper pour créer un slider avec son label
template<typename T>
void createSliderWithLabel(tgui::Gui& gui, const std::string& labelPrefix, 
                          T& variable, float x, float y, float min, float max,
                          float step, int precision = 0,
                          std::function<void()> callback = nullptr) {
    // Créer le texte du label
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision);
    if constexpr (std::is_integral_v<T>) {
        ss << static_cast<int>(variable);
    } else {
        ss << static_cast<float>(variable);
    }
    
    // Label
    auto label = createLabel(labelPrefix + ss.str(), x, y);
    gui.add(label);
    
    // Slider
    auto slider = tgui::Slider::create(min, max);
    slider->setPosition(x, y + 30);
    slider->setSize(250, 15);
    slider->setStep(step);
    slider->setValue(static_cast<float>(variable));
    gui.add(slider);
    
    // Callback
    slider->onValueChange([=, &variable]() mutable {
        if constexpr (std::is_integral_v<T>) {
            variable = static_cast<T>(slider->getValue());
            label->setText(labelPrefix + std::to_string(static_cast<int>(variable)));
        } else {
            variable = static_cast<T>(slider->getValue());
            std::stringstream ss;
            ss << std::fixed << std::setprecision(precision) << static_cast<float>(variable);
            label->setText(labelPrefix + ss.str());
        }
        
        if (callback) {
            callback();
        }
    });
}

void createParameter(tgui::Gui& gui) {
    leftSide(gui);
    rightSide(gui);
}

void leftSide(tgui::Gui& gui) {
    // Titre
    gui.add(createLabel("Paramètres généraux", 49, 65, 25, tgui::Color::White, true));
    
    // Dimension
    createSliderWithLabel(gui, "Dimension : ", game::map_size, 45, 115, 
                         5, 99, 2, 0, []() { createHexmap(); });

    // Seed climat
    createSliderWithLabel(gui, "Seed climat : ", game::climate_seed, 45, 195, 
                         0, 250, 1, 0, []() { createHexmap(); });
}

void rightSide(tgui::Gui& gui) {
    const float X_BASE = 1615;
    float yPos = 65;
    
    // Titre
    gui.add(createLabel("Carte", 1705, yPos, 25, tgui::Color::White, true));
    yPos += 50;
    
    // Section Aquatique
    gui.add(createLabel("----- Aquatique -----", X_BASE, yPos, 25));
    yPos += 50;
    
    // Seuil eau
    createSliderWithLabel(gui, "Seuil : ", game::water_threshold, X_BASE, yPos, 
                         0, 1, 0.025f, 2, []() { createHexmap(); });
    yPos += 80;
    
    // Seed eau
    createSliderWithLabel(gui, "Seed : ", game::water_seed, X_BASE, yPos, 
                         0, 250, 1, 0, []() { createHexmap(); });
    yPos += 80;
    
    // Octaves eau
    createSliderWithLabel(gui, "Octaves : ", game::water_octaves, X_BASE, yPos, 
                         1, 12, 1, 0, []() { createHexmap(); });
    yPos += 80;
    
    // Persistence eau
    createSliderWithLabel(gui, "Persistence : ", game::water_persistence, X_BASE, yPos, 
                         0, 5, 0.1f, 1, []() { createHexmap(); });
    yPos += 80;
    
    // Lacunarité eau
    createSliderWithLabel(gui, "Lacunarité : ", game::water_lacunarity, X_BASE, yPos, 
                         0, 12, 0.1f, 1, []() { createHexmap(); });
    yPos += 80;
    
    // Fréquence eau
    createSliderWithLabel(gui, "Fréquence : ", game::water_frequency, X_BASE, yPos, 
                         0.01f, 0.5f, 0.01f, 2, []() { createHexmap(); });
}