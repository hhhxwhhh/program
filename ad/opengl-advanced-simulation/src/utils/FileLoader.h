#ifndef FILELOADER_H
#define FILELOADER_H

#include <string>
#include <vector>

class FileLoader {
public:
    static std::vector<float> loadModel(const std::string& filePath);
    static unsigned int loadTexture(const std::string& filePath);
};

#endif // FILELOADER_H