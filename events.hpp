#pragma once

#include <GLFW/glfw3.h>
#include "rendering/Camera.hpp"

void initializeMouse(GLFWwindow* window);
void processEvents(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);