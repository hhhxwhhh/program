#include "Plane.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

Plane::Plane(float width, float height, unsigned int textureID) 
    : width(width), height(height), textureID(textureID) {
    setupPlane();
}

void Plane::setupPlane() {
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    std::vector<glm::vec3> vertices = {
        { -halfWidth, 0.0f, -halfHeight },
        { halfWidth, 0.0f, -halfHeight },
        { halfWidth, 0.0f, halfHeight },
        { -halfWidth, 0.0f, halfHeight }
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Plane::draw(Shader& shader) {
    shader.use();
    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Plane::~Plane() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}