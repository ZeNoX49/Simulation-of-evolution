#include "tileModel.hpp"

ObjData createTileModel(float x, float y, float h, float r, glm::vec3 color) {
    ObjData hex;

    glm::vec3 upNormal(0, 1, 0);

    // --- Centre de la face du dessus ---
    Vertex centerTop = { 
        glm::vec3(x, h, y), 
        upNormal, 
        glm::vec2(0.5f, 0.5f),
        color
    };
    hex.vertices.push_back(centerTop);

    std::vector<int> topIdx(6);
    std::vector<int> bottomIdx(6);

    float offsetAngle = glm::radians(30.0f);
    // --- Calcul des 6 sommets ---
    for (int i = 0; i < 6; i++) {
        float angle = glm::radians(60.f * i) + offsetAngle;
        float px = x + r * cos(angle);
        float pz = y + r * sin(angle);

        // Sommet du dessus
        Vertex top = { 
            glm::vec3(px, h, pz), 
            upNormal,
            glm::vec2((cos(angle) + 1.0f) * 0.5f, (sin(angle) + 1.0f) * 0.5f),
            color 
        };

        hex.vertices.push_back(top);
        topIdx[i] = hex.vertices.size() - 1;

        // Sommet du bas
        Vertex bottom = { 
            glm::vec3(px, 0.0f, pz),
            glm::normalize(glm::vec3(cos(angle), 0, sin(angle))), // normale mur
            glm::vec2(i / 6.0f, 1.0f),
            color
        };

        hex.vertices.push_back(bottom);
        bottomIdx[i] = hex.vertices.size() - 1;
    }

    // --- Face du dessus ---
    for (int i = 0; i < 6; i++) {
        int next = (i + 1) % 6;
        hex.indices.push_back(0);
        hex.indices.push_back(topIdx[i]);
        hex.indices.push_back(topIdx[next]);
    }

    // --- Murs latéraux ---
    for (int i = 0; i < 6; i++) {
        int next = (i + 1) % 6;

        // Triangle 1
        hex.indices.push_back(topIdx[i]);
        hex.indices.push_back(bottomIdx[i]);
        hex.indices.push_back(bottomIdx[next]);

        // Triangle 2
        hex.indices.push_back(topIdx[i]);
        hex.indices.push_back(bottomIdx[next]);
        hex.indices.push_back(topIdx[next]);
    }

    return hex;
}