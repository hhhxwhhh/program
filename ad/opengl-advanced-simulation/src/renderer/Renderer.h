#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"
#include "VertexArray.h"
#include "Camera.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init();
    void clear() const;
    void draw(const VertexArray& va, const Shader& shader, const glm::mat4& transform) const;

private:
    GLuint m_VAO;
    GLuint m_VBO;
    std::vector<GLuint> m_textures;
};

#endif // RENDERER_H