#include "RigidBody.h"

RigidBody::RigidBody(float mass, const glm::vec3& position)
    : mass(mass), position(position), velocity(0.0f), force(0.0f) {}

void RigidBody::applyForce(const glm::vec3& force) {
    this->force += force;
}

void RigidBody::update(float deltaTime) {
    if (mass > 0.0f) {
        glm::vec3 acceleration = force / mass;
        velocity += acceleration * deltaTime;
        position += velocity * deltaTime;
        force = glm::vec3(0.0f); // Reset force after each update
    }
}

glm::vec3 RigidBody::getPosition() const {
    return position;
}

glm::vec3 RigidBody::getVelocity() const {
    return velocity;
}

float RigidBody::getMass() const {
    return mass;
}