#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <GL/glew.h>

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    void use();
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;

private:
    unsigned int ID;

    void checkCompileErrors(unsigned int shader, const std::string& type);
    std::string loadShaderSource(const std::string& path);
};

#endif // SHADER_H