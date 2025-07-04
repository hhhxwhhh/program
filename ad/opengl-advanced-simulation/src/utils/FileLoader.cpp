#include "FileLoader.h"
#include <iostream>
#include <fstream>
#include <sstream>

std::string FileLoader::loadFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Could not open the file: " << filePath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

bool FileLoader::loadModel(const std::string& modelPath, Model& model) {
    // Implement model loading logic here
    // This is a placeholder for actual model loading code
    std::string modelData = loadFile(modelPath);
    if (modelData.empty()) {
        return false;
    }

    // Parse model data and populate the model object
    // ...

    return true;
}

bool FileLoader::loadTexture(const std::string& texturePath, Texture& texture) {
    // Implement texture loading logic here
    // This is a placeholder for actual texture loading code
    std::string textureData = loadFile(texturePath);
    if (textureData.empty()) {
        return false;
    }

    // Load texture data into the texture object
    // ...

    return true;
}