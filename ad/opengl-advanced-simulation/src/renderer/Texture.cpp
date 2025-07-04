#include "Texture.h"
#include <stb_image.h>
#include <glad/glad.h>
#include <iostream>

Texture::Texture(const std::string& path) 
    : m_Path(path), m_Width(0), m_Height(0), m_InternalFormat(GL_RGB), m_ImageFormat(GL_RGB), m_TextureID(0) {
    Load();
}

Texture::~Texture() {
    glDeleteTextures(1, &m_TextureID);
}

void Texture::Load() {
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(m_Path.c_str(), &m_Width, &m_Height, 0, 3);
    if (data) {
        glGenTextures(1, &m_TextureID);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_ImageFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cerr << "Failed to load texture at path: " << m_Path << std::endl;
        stbi_image_free(data);
    }
}

void Texture::Bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
}