#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H

#include "Light.h"
#include <glm/glm.hpp>

class DirectionalLight : public Light {
public:
    DirectionalLight(const glm::vec3& direction, const glm::vec3& color, float intensity);

    void setDirection(const glm::vec3& direction);
    void setIntensity(float intensity);
    
    glm::vec3 getDirection() const;
    float getIntensity() const;

private:
    glm::vec3 direction;
    float intensity;
};

#endif // DIRECTIONALLIGHT_H