#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include <vector>
#include "RigidBody.h"

class PhysicsEngine {
public:
    PhysicsEngine();
    ~PhysicsEngine();

    void addRigidBody(RigidBody* body);
    void removeRigidBody(RigidBody* body);
    void update(float deltaTime);

private:
    std::vector<RigidBody*> rigidBodies;
};

#endif // PHYSICS_ENGINE_H