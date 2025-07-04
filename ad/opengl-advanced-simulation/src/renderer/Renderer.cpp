#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

Renderer::Renderer() {
    // Initialization code here
}

Renderer::~Renderer() {
    // Cleanup code here
}

void Renderer::init() {
    // Initialize OpenGL settings
    glEnable(GL_DEPTH_TEST);
}

void Renderer::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::draw(const VertexArray& va, const Shader& shader, const Texture& texture) {
    shader.use();
    texture.bind();
    va.bind();
    
    glDrawArrays(GL_TRIANGLES, 0, va.getCount());
}

void Renderer::setViewport(int width, int height) {
    glViewport(0, 0, width, height);
}