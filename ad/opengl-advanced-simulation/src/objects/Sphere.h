#ifndef SPHERE_H
#define SPHERE_H

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

class Sphere {
public:
    Sphere(float radius, unsigned int rings, unsigned int sectors);
    void render();
    void setPosition(const glm::vec3& position);
    void setColor(const glm::vec3& color);

private:
    void generateSphere();
    
    float radius;
    unsigned int rings;
    unsigned int sectors;
    std::vector<glm::vec3> vertices;
    std::vector<GLuint> indices;
    GLuint VAO, VBO, EBO;
    glm::vec3 position;
    glm::vec3 color;
};

#endif // SPHERE_H