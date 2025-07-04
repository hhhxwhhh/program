#include "VertexArray.h"
#include <glad/glad.h>

VertexArray::VertexArray() {
    glGenVertexArrays(1, &m_ID);
}

VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &m_ID);
}

void VertexArray::Bind() const {
    glBindVertexArray(m_ID);
}

void VertexArray::Unbind() const {
    glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(const VertexBuffer& vertexBuffer) {
    Bind();
    vertexBuffer.Bind();
    // Assuming the VertexBuffer has a method to get the layout
    const auto& layout = vertexBuffer.GetLayout();
    for (const auto& element : layout) {
        glEnableVertexAttribArray(m_VertexBufferIndex);
        glVertexAttribPointer(m_VertexBufferIndex, element.count, element.type, element.normalized, layout.GetStride(), (const void*)element.offset);
        m_VertexBufferIndex++;
    }
}

void VertexArray::Clear() {
    m_VertexBufferIndex = 0;
}