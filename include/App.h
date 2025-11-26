#pragma once
#include "Renderer.h"
#include <GLFW/glfw3.h>


class App {
public:
    App();
    ~App();


    bool init();
    void run();
    void shutdown();


private:
    GLFWwindow* window = nullptr;
    Renderer renderer;
    int width = 1280;
    int height = 720;
};