#include "Light.h"

Light::Light() 
    : position(0.0f, 0.0f, 0.0f), color(1.0f, 1.0f, 1.0f), intensity(1.0f) {}

Light::Light(const glm::vec3& position, const glm::vec3& color, float intensity) 
    : position(position), color(color), intensity(intensity) {}

void Light::setPosition(const glm::vec3& position) {
    this->position = position;
}

void Light::setColor(const glm::vec3& color) {
    this->color = color;
}

void Light::setIntensity(float intensity) {
    this->intensity = intensity;
}

const glm::vec3& Light::getPosition() const {
    return position;
}

const glm::vec3& Light::getColor() const {
    return color;
}

float Light::getIntensity() const {
    return intensity;
}