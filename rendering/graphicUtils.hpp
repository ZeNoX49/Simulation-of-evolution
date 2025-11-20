#pragma once

#include "../object/ObjData.hpp"

void initObject(ObjData& obj);
void drawObject(ObjData& obj);
ObjData createSquare(float height, glm::vec3 color);