#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>

class Shader {
public:
    unsigned int ID = 0;
    Shader() = default;
    ~Shader();
    bool loadFromFiles(const char* vertPath, const char* fragPath);
    void use() const;

    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, const glm::vec3 &v) const;
};
