#ifndef CUBE_H
#define CUBE_H

#include <glm/glm.hpp>
#include <vector>

class Cube {
public:
    Cube();
    void draw();
    void setPosition(const glm::vec3& position);
    void setScale(const glm::vec3& scale);
    void setRotation(float angle, const glm::vec3& axis);
    
private:
    void setupMesh();
    
    unsigned int VAO, VBO;
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotationAxis;
    float rotationAngle;
    std::vector<float> vertices;
};

#endif // CUBE_H