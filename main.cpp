#include <chrono>
#include <iostream>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "events.hpp"
#include "configuration.hpp"
#include "gameParam.hpp"
#include "environment/map.hpp"
#include "rendering/graphicUtils.hpp"
#include "rendering/guiParameter.hpp"

// Fonction callback pour redimensionner la fenêtre
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    resize_window(width, height);
}

unsigned int compileShader();

int main() {
     // Initialiser GLFW
    if (!glfwInit()) {
        std::cerr << "Erreur d'initialisation de GLFW\n";
        return -1;
    }

    // Spécifier la version OpenGL (ici 3.3 Core)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    resize_window(mode->width, mode->height);

    // Créer la fenêtre en plein écran
    GLFWwindow* window = glfwCreateWindow(conf::width, conf::height, "Simulation de l'évolution - OpenGL", primaryMonitor, NULL);
    if (!window) {
        std::cerr << "Erreur de création de fenêtre\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialiser GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Erreur d'initialisation de GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST); // pour voir cube correctement
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); // par défaut

    unsigned int shaderProgram = compileShader();
    
    initializeMouse(window);

    loadResources();

    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark(); // Thème par défaut

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    // Générer la carte initiale
    createHexmap();

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processEvents(window);

        // Démarrer un nouveau frame ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        createParameter();

        // Couleur de fond
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Passer les matrices au shader
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = gameUtils::cam.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(gameUtils::cam.focalLenth), conf::width_f / conf::height_f, 0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        for (ObjData& tile : map::hexmap_drawable) {
            drawObject(tile);
        }

        if(gameParam::showWaterLevel) {
            float height = gameParam::water_threshold;
            if(gameParam::tile_color != 0) height *= 5;
            ObjData waterLevelSquare = createSquare(height + (1.0f / conf::model_size_div), glm::vec3(0, 0, 220));
            drawObject(waterLevelSquare);
        }
        if(gameParam::showMaxHeight) {
            float height = 1.0f;
            if(gameParam::tile_color != 0) height *= 5;
            ObjData maxHeightSquare = createSquare(height + (1.0f / conf::model_size_div), glm::vec3(150, 150, 150));
            drawObject(maxHeightSquare);
        }

        // Rendu de l’UI ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    return 0;
}

unsigned int compileShader() {
    // 1. Charger le code source depuis les fichiers
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile("shaders/vertex.glsl");
    std::ifstream fShaderFile("shaders/fragment.glsl");

    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();
    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();
    std::string vShaderCodeStr = vertexCode;
    std::string fShaderCodeStr = fragmentCode;
    const char* vShaderCode = vShaderCodeStr.c_str();
    const char* fShaderCode = fShaderCodeStr.c_str();

    // 2. Compiler le vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);

    // Vérifier les erreurs
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "Erreur compilation vertex shader:\n" << infoLog << std::endl;
    }

    // 3. Compiler le fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);

    // Vérifier les erreurs
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "Erreur compilation fragment shader:\n" << infoLog << std::endl;
    }

    // 4. Lier les shaders dans un programme
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Vérifier les erreurs
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "Erreur linking shader program:\n" << infoLog << std::endl;
    }

    // 5. Supprimer les shaders compilés (plus besoin d'eux)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}