#pragma once
#include <string>
#include <glad/glad.h>


class Shader {
public:
    Shader();
    ~Shader();


    bool loadFromFiles(const std::string& vsPath, const std::string& fsPath);
    bool loadComputeFromFile(const std::string& csPath);


    GLuint getID() const { return programID; }

    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setVec3(const std::string& name, float x, float y, float z) const;


private:
    GLuint programID = 0;
    bool compileShader(const char* src, GLenum type, GLuint& out);
    std::string readFile(const std::string& path);
};