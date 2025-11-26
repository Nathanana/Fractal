#include "App.h"
#include <iostream>
#include <glad/glad.h>

App::App() {}
App::~App() {}

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
}

bool App::init() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "FractalRenderer", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return false;
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    // THESE LINES ARE CORRECT FOR MOUSE LOOK:
    glfwSetWindowUserPointer(window, &renderer);
    glfwSetCursorPosCallback(window, Renderer::mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hides and captures mouse

    if (!renderer.init(width, height)) {
        std::cerr << "Renderer failed to initialize!" << std::endl;
        return false;
    }

    std::cout << "Initialization successful." << std::endl;
    return true;
}

void App::run() {
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = float(now - lastTime);
        lastTime = now;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

        renderer.update(dt, window);
        renderer.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void App::shutdown() {
    renderer.shutdown();
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
}