#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <GL/glew.h>

class Texture {
public:
    Texture(const std::string& filePath);
    ~Texture();

    void bind(unsigned int slot = 0) const;
    void unbind() const;

private:
    GLuint textureID;
    std::string filePath;
    int width, height, numChannels;

    void loadTexture();
};

#endif // TEXTURE_H