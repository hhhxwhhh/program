#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "Light.h"
#include <glm/glm.hpp>

class PointLight : public Light {
public:
    PointLight(const glm::vec3& position, const glm::vec3& color, float intensity);

    void setPosition(const glm::vec3& position);
    void setIntensity(float intensity);
    glm::vec3 getPosition() const;
    float getIntensity() const;

private:
    glm::vec3 position;
    float intensity;
};

#endif // POINTLIGHT_H