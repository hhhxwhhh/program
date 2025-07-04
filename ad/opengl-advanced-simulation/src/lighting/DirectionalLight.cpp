#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(const glm::vec3& direction, const glm::vec3& color, float intensity)
    : Light(color, intensity), direction(direction) {}

void DirectionalLight::setDirection(const glm::vec3& dir) {
    direction = dir;
}

glm::vec3 DirectionalLight::getDirection() const {
    return direction;
}

void DirectionalLight::applyLight(Shader& shader) const {
    shader.setVec3("directionalLight.direction", direction);
    shader.setVec3("directionalLight.color", color);
    shader.setFloat("directionalLight.intensity", intensity);
}