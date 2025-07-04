#ifndef RIGIDBODY_H
#define RIGIDBODY_H

class RigidBody {
public:
    RigidBody(float mass);
    void applyForce(const glm::vec3& force);
    void update(float deltaTime);
    void setMass(float mass);
    float getMass() const;
    glm::vec3 getPosition() const;
    glm::vec3 getVelocity() const;

private:
    float mass;
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
};

#endif // RIGIDBODY_H