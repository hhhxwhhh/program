// 使用绝对路径
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// 或者如果你不想更改源代码，确保在编译时使用正确的包含路径
#include <D:\OpenGL\program\glm\glm.hpp>
#include <iostream>

int main()
{
    // 创建一个三维向量
    glm::vec3 position(1.0f, 0.0f, 0.0f);

    // 创建变换矩阵
    glm::mat4 transform = glm::mat4(1.0f); // 单位矩阵

    // 旋转90度
    transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    // 应用变换
    glm::vec4 result = transform * glm::vec4(position, 1.0f);

    // 输出结果
    std::cout << "原始坐标: (" << position.x << ", " << position.y << ", " << position.z << ")\n";
    std::cout << "变换后坐标: (" << result.x << ", " << result.y << ", " << result.z << ")\n";
    system("pause");
    return 0;
}