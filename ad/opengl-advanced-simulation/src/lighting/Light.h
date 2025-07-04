#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

class Light {
public:
    Light(const glm::vec3& position, const glm::vec3& color);
    
    void setPosition(const glm::vec3& position);
    void setColor(const glm::vec3& color);
    
    glm::vec3 getPosition() const;
    glm::vec3 getColor() const;

private:
    glm::vec3 position;
    glm::vec3 color;
};

#endif // LIGHT_H