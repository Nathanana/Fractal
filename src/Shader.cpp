#include "shader.h"
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>

static std::string readFile(const char* path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Failed to open shader file: " << path << "\n";
        return {};
    }
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

Shader::~Shader() {
    if (ID) glDeleteProgram(ID);
}

static unsigned int compileShader(unsigned int type, const char* src) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int success = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetShaderInfoLog(id, 1024, nullptr, info);
        std::cerr << "Shader compile error: " << info << "\n";
        glDeleteShader(id);
        return 0;
    }
    return id;
}

bool Shader::loadFromFiles(const char* vertPath, const char* fragPath) {
    std::string vcode = readFile(vertPath);
    std::string fcode = readFile(fragPath);
    if (vcode.empty() || fcode.empty()) return false;

    unsigned int vs = compileShader(GL_VERTEX_SHADER, vcode.c_str());
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fcode.c_str());
    if (!vs || !fs) return false;

    ID = glCreateProgram();
    glAttachShader(ID, vs);
    glAttachShader(ID, fs);
    glLinkProgram(ID);

    int linked = 0;
    glGetProgramiv(ID, GL_LINK_STATUS, &linked);
    if (!linked) {
        char info[1024];
        glGetProgramInfoLog(ID, 1024, nullptr, info);
        std::cerr << "Program link error: " << info << "\n";
        glDeleteProgram(ID);
        ID = 0;
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    glDetachShader(ID, vs);
    glDetachShader(ID, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return true;
}

void Shader::use() const {
    if (ID) glUseProgram(ID);
}

void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setVec3(const std::string &name, const glm::vec3 &v) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), v.x, v.y, v.z);
}
