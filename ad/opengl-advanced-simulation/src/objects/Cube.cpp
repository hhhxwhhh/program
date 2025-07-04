#include "Cube.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Cube::Cube() {
    // Define the vertices for a cube
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  // Back bottom left
         0.5f, -0.5f, -0.5f,  // Back bottom right
         0.5f,  0.5f, -0.5f,  // Back top right
         0.5f,  0.5f, -0.5f,  // Back top right
        -0.5f,  0.5f, -0.5f,  // Back top left
        -0.5f, -0.5f, -0.5f,  // Back bottom left

        -0.5f, -0.5f,  0.5f,  // Front bottom left
         0.5f, -0.5f,  0.5f,  // Front bottom right
         0.5f,  0.5f,  0.5f,  // Front top right
         0.5f,  0.5f,  0.5f,  // Front top right
        -0.5f,  0.5f,  0.5f,  // Front top left
        -0.5f, -0.5f,  0.5f,  // Front bottom left

        -0.5f,  0.5f,  0.5f,  // Front top left
        -0.5f,  0.5f, -0.5f,  // Back top left
        -0.5f, -0.5f, -0.5f,  // Back bottom left
        -0.5f, -0.5f, -0.5f,  // Back bottom left
        -0.5f, -0.5f,  0.5f,  // Front bottom left
        -0.5f,  0.5f,  0.5f,  // Front top left

         0.5f,  0.5f,  0.5f,  // Front top right
         0.5f,  0.5f, -0.5f,  // Back top right
         0.5f, -0.5f, -0.5f,  // Back bottom right
         0.5f, -0.5f, -0.5f,  // Back bottom right
         0.5f, -0.5f,  0.5f,  // Front bottom right
         0.5f,  0.5f,  0.5f,  // Front top right

        -0.5f, -0.5f, -0.5f,  // Back bottom left
         0.5f, -0.5f, -0.5f,  // Back bottom right
         0.5f, -0.5f,  0.5f,  // Front bottom right
         0.5f, -0.5f,  0.5f,  // Front bottom right
        -0.5f, -0.5f,  0.5f,  // Front bottom left
        -0.5f, -0.5f, -0.5f,  // Back bottom left

        -0.5f,  0.5f, -0.5f,  // Back top left
         0.5f,  0.5f, -0.5f,  // Back top right
         0.5f,  0.5f,  0.5f,  // Front top right
         0.5f,  0.5f,  0.5f,  // Front top right
        -0.5f,  0.5f,  0.5f,  // Front top left
        -0.5f,  0.5f, -0.5f   // Back top left
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Cube::Draw(Shader& shader) {
    shader.use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

Cube::~Cube() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}