#ifndef PLANE_H
#define PLANE_H

#include <vector>
#include <GL/glew.h>
#include "Shader.h"

class Plane {
public:
    Plane(float width, float depth, unsigned int rows, unsigned int cols);
    void draw(Shader& shader);

private:
    void setupMesh();
    
    float width;
    float depth;
    unsigned int rows;
    unsigned int cols;
    GLuint VAO, VBO;
    std::vector<float> vertices;
};

#endif // PLANE_H