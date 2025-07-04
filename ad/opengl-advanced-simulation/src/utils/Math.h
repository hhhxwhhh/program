#ifndef MATH_H
#define MATH_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Math {

    glm::vec3 Normalize(const glm::vec3& vector) {
        return glm::normalize(vector);
    }

    float Dot(const glm::vec3& a, const glm::vec3& b) {
        return glm::dot(a, b);
    }

    glm::vec3 Cross(const glm::vec3& a, const glm::vec3& b) {
        return glm::cross(a, b);
    }

    glm::mat4 Translate(const glm::mat4& matrix, const glm::vec3& translation) {
        return glm::translate(matrix, translation);
    }

    glm::mat4 Rotate(const glm::mat4& matrix, float angle, const glm::vec3& axis) {
        return glm::rotate(matrix, glm::radians(angle), axis);
    }

    glm::mat4 Scale(const glm::mat4& matrix, const glm::vec3& scale) {
        return glm::scale(matrix, scale);
    }

    glm::mat4 CreateViewMatrix(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
        return glm::lookAt(position, target, up);
    }

    glm::mat4 CreateProjectionMatrix(float fov, float aspect, float near, float far) {
        return glm::perspective(glm::radians(fov), aspect, near, far);
    }

}

#endif // MATH_H