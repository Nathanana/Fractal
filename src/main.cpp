#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"

#include <iostream>
#include <cmath>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int maxIterations = 16;
bool autoRotate = false;

glm::vec3 cameraMantissa = glm::vec3(0.0f, 0.0f, 5.0f);
int cameraExponent = 0;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Fractal Explorer - Mandelbulb", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    std::cout << "Controls:" << std::endl;
    std::cout << "WASD - Move horizontally" << std::endl;
    std::cout << "Space/Shift - Move up/down" << std::endl;
    std::cout << "Mouse - Look around" << std::endl;
    std::cout << "Scroll - Adjust speed" << std::endl;
    std::cout << "Q/E - Zoom in/out" << std::endl;
    std::cout << "1/2 - Decrease/Increase iterations" << std::endl;
    std::cout << "R - Toggle auto-rotate" << std::endl; 
    std::cout << "ESC - Exit" << std::endl;
    std::cout << "\nStarting iterations: " << maxIterations << std::endl;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        std::cout << "\rFPS: " << static_cast<int>(1.0f / deltaTime)
          << " | Zoom Level: 2^" << cameraExponent
          << " | Max Iterations: " << maxIterations
          << " | Speed: " << camera.MovementSpeed
          << std::flush;

        processInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();

        float scale = std::pow(2.0f, static_cast<float>(cameraExponent));
        glm::vec3 worldPos = cameraMantissa * scale;

        shader.setVec3("camPos", worldPos);
        shader.setVec3("camFront", camera.Front);
        shader.setVec3("camRight", camera.Right);
        shader.setVec3("camUp", camera.Up);
        shader.setFloat("fov", glm::radians(camera.Fov));
        shader.setFloat("time", currentFrame);
        shader.setFloat("scale", scale); 

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        shader.setVec3("resolution", glm::vec3(width, height, 0.0f));

        shader.setInt("maxIterations", maxIterations);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec3 oldMantissa = cameraMantissa;

    float velocity = camera.MovementSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraMantissa += camera.Front * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraMantissa -= camera.Front * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraMantissa -= camera.Right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraMantissa += camera.Right * velocity;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraMantissa += camera.Up * velocity;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraMantissa -= camera.Up * velocity;

    float mantissaLen = glm::length(cameraMantissa);
    if (mantissaLen > 10.0f) {
        cameraMantissa /= 2.0f;
        cameraExponent += 1;
    } else if (mantissaLen < 0.1f && mantissaLen > 0.001f) {
        cameraMantissa *= 2.0f;
        cameraExponent -= 1;
    }

    static bool key1Pressed = false;
    static bool key2Pressed = false;
    static bool keyQPressed = false;
    static bool keyEPressed = false;

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !keyEPressed) {
        cameraExponent += 1;
        camera.MinSpeed /= 2.0f;
        camera.MaxSpeed /= 2.0f;
        camera.MovementSpeed /= 2.0f;
        
        keyEPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE)
        keyEPressed = false;

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && !keyQPressed) {
        cameraExponent -= 1;
        camera.MinSpeed *= 2.0f;
        camera.MaxSpeed *= 2.0f;
        camera.MovementSpeed *= 2.0f;
        
        keyQPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE)
        keyQPressed = false;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !key1Pressed) {
        maxIterations = std::max(0, maxIterations - 1);
        
        key1Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE)
        key1Pressed = false;

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !key2Pressed) {
        maxIterations = std::min(256, maxIterations + 1);
        
        key2Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE)
        key2Pressed = false;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.processMouseScroll(static_cast<float>(yoffset));
}