#include "PointLight.h"

PointLight::PointLight(glm::vec3 position, glm::vec3 color, float intensity)
    : Light(color, intensity), position(position) {}

void PointLight::setPosition(const glm::vec3& newPosition) {
    position = newPosition;
}

glm::vec3 PointLight::getPosition() const {
    return position;
}

void PointLight::setIntensity(float newIntensity) {
    intensity = newIntensity;
}

float PointLight::getIntensity() const {
    return intensity;
}

void PointLight::applyLight(Shader& shader) const {
    shader.setVec3("pointLight.position", position);
    shader.setVec3("pointLight.color", color);
    shader.setFloat("pointLight.intensity", intensity);
}