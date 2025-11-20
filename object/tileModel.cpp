#include "tileModel.hpp"
#include <glm/ext/matrix_transform.hpp>
#include "../gameParam.hpp"

ObjData createTileModel(float x, float y, float h, float r, glm::vec3 color) {
    // Copie du modèle de base
    ObjData tile = tileModelOriginal;

    // Mise à jour de la couleur sur chaque vertex
    for(auto& v : tile.vertices) {
        v.position *= r;
        v.position += glm::vec3(x, h, y);
        v.color = color;

        if(v.position.y < h) {
            if(gameParam::tile_color == 0) {
                v.position.y = 0.0f;
            } else {
                v.position = glm::vec3(x, h, y);
            }
        }
    }

    return tile;
}

ObjData loadOBJ(const std::string& path) {
    ObjData objData;
    
    std::ifstream file(path);
    if (!file.is_open()) {
        std::printf("Impossible d'ouvrir le fichier : %s\n", path.c_str());
        return objData;
    }
    
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec3> temp_normals;
    std::vector<glm::vec2> temp_uvs;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") {
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);
        }
        else if (type == "vt") {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (type == "vn") {
            glm::vec3 normal;
            ss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (type == "f") {
            std::string vertexString;
            std::vector<unsigned int> faceIndices;

            while (ss >> vertexString) {
                unsigned int vIdx = 0, tIdx = 0, nIdx = 0;
                sscanf(vertexString.c_str(), "%u/%u/%u", &vIdx, &tIdx, &nIdx);

                Vertex vertex;
                vertex.position = temp_positions[vIdx - 1];
                vertex.texCoords = (tIdx > 0) ? temp_uvs[tIdx - 1] : glm::vec2(0.0f);
                vertex.normal = (nIdx > 0) ? temp_normals[nIdx - 1] : glm::vec3(0.0f, 0.0f, 1.0f);
                objData.vertices.push_back(vertex);

                faceIndices.push_back(objData.vertices.size() - 1);
            }

            // Trianguler les faces si elles ont plus de 3 sommets
            for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                objData.indices.push_back(faceIndices[0]);
                objData.indices.push_back(faceIndices[i]);
                objData.indices.push_back(faceIndices[i + 1]);
            }
        }
    }

    // std::printf("OBJ chargé : %zu sommets, %zu triangles\n", objData.vertices.size(), objData.indices.size() / 3);

    return objData;
}

// ObjData createTileModel(float x, float y, float h, float r, glm::vec3 color) {
//     ObjData hex;

//     glm::vec3 upNormal(0, 1, 0);

//     // --- Centre de la face du dessus ---
//     Vertex centerTop = { 
//         glm::vec3(x, h, y), 
//         upNormal, 
//         glm::vec2(0.5f, 0.5f),
//         color
//     };
//     hex.vertices.push_back(centerTop);

//     std::vector<int> topIdx(6);
//     std::vector<int> bottomIdx(6);

//     float offsetAngle = glm::radians(30.0f);
//     // --- Calcul des 6 sommets ---
//     for (int i = 0; i < 6; i++) {
//         float angle = glm::radians(60.f * i) + offsetAngle;
//         float px = x + r * cos(angle);
//         float pz = y + r * sin(angle);

//         // Sommet du dessus
//         Vertex top = { 
//             glm::vec3(px, h, pz), 
//             upNormal,
//             glm::vec2((cos(angle) + 1.0f) * 0.5f, (sin(angle) + 1.0f) * 0.5f),
//             color 
//         };

//         hex.vertices.push_back(top);
//         topIdx[i] = hex.vertices.size() - 1;

//         // Sommet du bas
//         Vertex bottom = { 
//             glm::vec3(px, 0.0f, pz),
//             glm::normalize(glm::vec3(cos(angle), 0, sin(angle))), // normale mur
//             glm::vec2(i / 6.0f, 1.0f),
//             color
//         };

//         hex.vertices.push_back(bottom);
//         bottomIdx[i] = hex.vertices.size() - 1;
//     }

//     // --- Face du dessus ---
//     for (int i = 0; i < 6; i++) {
//         int next = (i + 1) % 6;
//         hex.indices.push_back(0);
//         hex.indices.push_back(topIdx[i]);
//         hex.indices.push_back(topIdx[next]);
//     }

//     // --- Murs latéraux ---
//     for (int i = 0; i < 6; i++) {
//         int next = (i + 1) % 6;

//         // Triangle 1
//         hex.indices.push_back(topIdx[i]);
//         hex.indices.push_back(bottomIdx[i]);
//         hex.indices.push_back(bottomIdx[next]);

//         // Triangle 2
//         hex.indices.push_back(topIdx[i]);
//         hex.indices.push_back(bottomIdx[next]);
//         hex.indices.push_back(topIdx[next]);
//     }

//     return hex;
// }