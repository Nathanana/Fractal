#pragma once
#include "Shader.h"
#include <GLFW/glfw3.h>
#include <cmath> // Required for trigonometric functions

class Renderer {
public:
    Renderer();
    ~Renderer();


    bool init(int width, int height);
    void update(float dt, GLFWwindow* window); 
    void render();
    void shutdown();

    // FIX: MOVED to public so GLFW can access it
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);


private:
    int width = 1280;
    int height = 720;


    GLuint texOutput = 0;
    GLuint vao = 0, vbo = 0;


    Shader computeShader;
    Shader blitShader;


    float time = 0.0f;
    float fractalLod = 1.0f;
    
    struct Camera {
        float pos[3] = {0.0f, 0.0f, -4.0f}; 
        float speed = 2.0f; 
        
        float yaw = 90.0f; 
        float pitch = 0.0f; 
        float sensitivity = 0.1f;
        
        float forward[3] = {0.0f, 0.0f, -1.0f};
        float up[3] = {0.0f, 1.0f, 0.0f}; 
        float right[3] = {1.0f, 0.0f, 0.0f}; 
    } camera;

    double lastX = 0.0;
    double lastY = 0.0;
    bool firstMouse = true;

    void updateCameraVectors();
};