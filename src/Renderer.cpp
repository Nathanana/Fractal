#include "Renderer.h"
#include <glad/glad.h>
#include <iostream>
#include <cmath> 
#include <algorithm> 

#define DEG_TO_RAD (3.14159265358979323846f / 180.0f)


Renderer::Renderer() {}
Renderer::~Renderer() {}

void Renderer::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    Renderer* self = static_cast<Renderer*>(glfwGetWindowUserPointer(window));

    if (self->firstMouse) {
        self->lastX = xpos;
        self->lastY = ypos;
        self->firstMouse = false;
    }

    float xoffset = (float)(xpos - self->lastX);
    float yoffset = (float)(self->lastY - ypos);
    self->lastX = xpos;
    self->lastY = ypos;

    xoffset *= self->camera.sensitivity;
    yoffset *= self->camera.sensitivity;

    self->camera.yaw   -= xoffset;
    self->camera.pitch -= yoffset;

    self->camera.pitch = std::clamp(self->camera.pitch, -89.0f, 89.0f);
    
    self->updateCameraVectors();
}

void Renderer::updateCameraVectors() {
    float yawRad = camera.yaw * DEG_TO_RAD;
    float pitchRad = camera.pitch * DEG_TO_RAD;

    camera.forward[0] = std::cos(yawRad) * std::cos(pitchRad);
    camera.forward[1] = std::sin(pitchRad);
    camera.forward[2] = std::sin(yawRad) * std::cos(pitchRad);

    float len = std::sqrt(camera.forward[0]*camera.forward[0] + camera.forward[1]*camera.forward[1] + camera.forward[2]*camera.forward[2]);
    if (len > 1e-6) {
        for(int i = 0; i < 3; ++i) camera.forward[i] /= len;
    }

    camera.right[0] = camera.forward[2];
    camera.right[1] = 0.0f;
    camera.right[2] = -camera.forward[0];

    len = std::sqrt(camera.right[0]*camera.right[0] + camera.right[1]*camera.right[1] + camera.right[2]*camera.right[2]);
    if (len > 1e-6) {
        for(int i = 0; i < 3; ++i) camera.right[i] /= len;
    }

    camera.up[0] = camera.right[1] * camera.forward[2] - camera.right[2] * camera.forward[1];
    camera.up[1] = camera.right[2] * camera.forward[0] - camera.right[0] * camera.forward[2];
    camera.up[2] = camera.right[0] * camera.forward[1] - camera.right[1] * camera.forward[0];
}


bool Renderer::init(int w, int h) {
    width = w; height = h;

    updateCameraVectors(); 

    glGenTextures(1, &texOutput);
    glBindTexture(GL_TEXTURE_2D, texOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    if (glGetError() != GL_NO_ERROR) std::cerr << "Texture creation failed!" << std::endl;
    glBindTexture(GL_TEXTURE_2D, 0);

    float quad[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    if (!computeShader.loadComputeFromFile("shaders/fractal.comp")) {
        std::cerr << "Failed to load compute shader!" << std::endl;
        return false;
    }
    if (!blitShader.loadFromFiles("shaders/quad.vert", "shaders/quad.frag")) {
        std::cerr << "Failed to load blit shader!" << std::endl;
        return false;
    }

    std::cout << "Renderer initialized successfully." << std::endl;
    return true;
}

void Renderer::update(float dt, GLFWwindow* window) {
    time += dt;
    fractalLod = 1.0f + std::sin(time * 0.3f) * 0.5f;

    float cameraSpeed = camera.speed * dt;
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        for(int i = 0; i < 3; ++i) camera.pos[i] += cameraSpeed * camera.forward[i];
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        for(int i = 0; i < 3; ++i) camera.pos[i] -= cameraSpeed * camera.forward[i];
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        for(int i = 0; i < 3; ++i) camera.pos[i] -= cameraSpeed * camera.right[i];
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        for(int i = 0; i < 3; ++i) camera.pos[i] += cameraSpeed * camera.right[i];
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.pos[1] += cameraSpeed; 
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera.pos[1] -= cameraSpeed; 
    }
}


void Renderer::render() {
    glUseProgram(computeShader.getID());

    glBindImageTexture(0, texOutput, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    computeShader.setVec2("resolution", (float)width, (float)height);
    computeShader.setFloat("time", time);
    computeShader.setFloat("fractalLod", fractalLod);

    computeShader.setVec3("rayPos", camera.pos[0], camera.pos[1], camera.pos[2]); 
    computeShader.setVec3("rayDir", camera.forward[0], camera.forward[1], camera.forward[2]); 
    computeShader.setVec3("camUp", camera.up[0], camera.up[1], camera.up[2]); 
    computeShader.setVec3("camRight", camera.right[0], camera.right[1], camera.right[2]); 


    int groupX = (width + 7) / 8;
    int groupY = (height + 7) / 8;
    glDispatchCompute(groupX, groupY, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(blitShader.getID());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texOutput);
    blitShader.setInt("tex", 0);


    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


void Renderer::shutdown() {
    if (texOutput) glDeleteTextures(1, &texOutput);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}