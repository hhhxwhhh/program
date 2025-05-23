#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

int main()
{
    // Create a 3D vector
    glm::vec3 position(1.0f, 0.0f, 0.0f);

    // Create transformation matrix
    glm::mat4 transform = glm::mat4(1.0f); // Identity matrix

    // Rotate 90 degrees
    transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    // Apply transformation
    glm::vec4 result = transform * glm::vec4(position, 1.0f);

    // Output results
    std::cout << "Original position: (" << position.x << ", " << position.y << ", " << position.z << ")\n";
    std::cout << "Transformed position: (" << result.x << ", " << result.y << ", " << result.z << ")\n";
    std::cout << "Press Enter to continue...";
    std::cin.get();
    return 0;
}