#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include <GL/glew.h>
#include <vector>

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    void bind() const;
    void unbind() const;

    void addVertexBuffer(unsigned int vertexBufferID);
    void setIndexBuffer(unsigned int indexBufferID);

private:
    unsigned int m_RendererID;
    std::vector<unsigned int> m_VertexBuffers;
    unsigned int m_IndexBuffer;
};

#endif // VERTEXARRAY_H