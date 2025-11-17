#include "graphicUtils.hpp"
#include "../configuration.hpp"

#include <glad/glad.h>

void initObject(ObjData& obj) {
    glGenVertexArrays(1, &obj.VAO);
    glGenBuffers(1, &obj.VBO);
    glGenBuffers(1, &obj.EBO);

    glBindVertexArray(obj.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, obj.VBO);
    glBufferData(GL_ARRAY_BUFFER, obj.vertices.size() * sizeof(Vertex), obj.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj.indices.size() * sizeof(unsigned int), obj.indices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    // Normale
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    // Coordonnée texture
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(2);
    // Couleur
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
}

void drawObject(ObjData& obj) {
    glBindVertexArray(obj.VAO);
    glDrawElements(GL_TRIANGLES, obj.indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

/* -------------------------------------------------------------- */

ObjData createSquare(float height, glm::vec3 color) {
    ObjData obj;
    float mult = 50.0f;

    float size = ((conf::game_window_width * 1.1f) / 2.0f) / mult;

    // Centre du carré
    float center = (conf::game_window_width / 2.0f) / mult;

    obj.vertices = {
        // position                                      normal    uv       color
        {{center - size, height, center - size}, {0,1,0}, {0,0}, color},
        {{center + size, height, center - size}, {0,1,0}, {1,0}, color},
        {{center + size, height, center + size}, {0,1,0}, {1,1}, color},
        {{center - size, height, center + size}, {0,1,0}, {0,1}, color},
    };

    obj.indices = {
        0, 1, 2,
        2, 3, 0
    };

    initObject(obj);
    return obj;
}