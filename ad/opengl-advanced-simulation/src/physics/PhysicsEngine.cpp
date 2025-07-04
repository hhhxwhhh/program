#include "PhysicsEngine.h"
#include "RigidBody.h"
#include <vector>

class PhysicsEngine {
public:
    PhysicsEngine();
    ~PhysicsEngine();

    void Update(float deltaTime);
    void AddRigidBody(RigidBody* body);
    void RemoveRigidBody(RigidBody* body);

private:
    std::vector<RigidBody*> rigidBodies;
};

PhysicsEngine::PhysicsEngine() {}

PhysicsEngine::~PhysicsEngine() {
    for (auto body : rigidBodies) {
        delete body;
    }
}

void PhysicsEngine::Update(float deltaTime) {
    for (auto body : rigidBodies) {
        body->Update(deltaTime);
    }
}

void PhysicsEngine::AddRigidBody(RigidBody* body) {
    rigidBodies.push_back(body);
}

void PhysicsEngine::RemoveRigidBody(RigidBody* body) {
    rigidBodies.erase(std::remove(rigidBodies.begin(), rigidBodies.end(), body), rigidBodies.end());
}