#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader() {}
Shader::~Shader() {
    if (programID) glDeleteProgram(programID);
}


std::string Shader::readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return "";
    }
    std::stringstream ss; ss << in.rdbuf();
    return ss.str();
}

bool Shader::compileShader(const char* src, GLenum type, GLuint& out) {
    out = glCreateShader(type);
    glShaderSource(out, 1, &src, nullptr);
    glCompileShader(out);
    GLint ok = 0; glGetShaderiv(out, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0; glGetShaderiv(out, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetShaderInfoLog(out, len, nullptr, log.data());
        std::cerr << "Shader compile error: " << log << std::endl;
        return false;
    }
    return true;
}

bool Shader::loadFromFiles(const std::string& vsPath, const std::string& fsPath) {
    std::string vsSrc = readFile(vsPath);
    std::string fsSrc = readFile(fsPath);
    if (vsSrc.empty() || fsSrc.empty()) return false;


    GLuint vs, fs;
    if (!compileShader(vsSrc.c_str(), GL_VERTEX_SHADER, vs)) return false;
    if (!compileShader(fsSrc.c_str(), GL_FRAGMENT_SHADER, fs)) { glDeleteShader(vs); return false; }


    programID = glCreateProgram();
    glAttachShader(programID, vs);
    glAttachShader(programID, fs);
    glLinkProgram(programID);


    GLint ok = 0; glGetProgramiv(programID, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0; glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetProgramInfoLog(programID, len, nullptr, log.data());
        std::cerr << "Program link error: " << log << std::endl;
        glDeleteShader(vs); glDeleteShader(fs);
        return false;
    }


    glDeleteShader(vs); glDeleteShader(fs);
    return true;
}

bool Shader::loadComputeFromFile(const std::string& csPath) {
    std::string csSrc = readFile(csPath);
    if (csSrc.empty()) return false;


    GLuint cs;
    if (!compileShader(csSrc.c_str(), GL_COMPUTE_SHADER, cs)) return false;


    programID = glCreateProgram();
    glAttachShader(programID, cs);
    glLinkProgram(programID);


    GLint ok = 0; glGetProgramiv(programID, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0; glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetProgramInfoLog(programID, len, nullptr, log.data());
        std::cerr << "Compute program link error: " << log << std::endl;
        glDeleteShader(cs);
    return false;
    }


    glDeleteShader(cs);
    return true;
}


void Shader::setInt(const std::string& name, int value) const {
    GLint loc = glGetUniformLocation(programID, name.c_str());
    if (loc != -1) glUniform1i(loc, value);
}

void Shader::setFloat(const std::string& name, float value) const {
    GLint loc = glGetUniformLocation(programID, name.c_str());
    if (loc != -1) glUniform1f(loc, value);
}

void Shader::setVec2(const std::string& name, float x, float y) const {
    GLint loc = glGetUniformLocation(programID, name.c_str());
    if (loc != -1) glUniform2f(loc, x, y);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const {
    GLint loc = glGetUniformLocation(programID, name.c_str());
    if (loc != -1) glUniform3f(loc, x, y, z);
}