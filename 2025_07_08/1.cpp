// 使用现代OpenGL（GLFW+GLAD）绘制三维坐标系
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// 顶点着色器源码
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 MVP;
void main() {
    gl_Position = MVP * vec4(aPos, 1.0);
}
)";

// 片段着色器源码
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
}
)";

// 三维坐标轴顶点数据
float axesVertices[] = {
    // X轴
    0.0f, 0.0f, 0.0f,  2.0f, 0.0f, 0.0f,
    // Y轴
    0.0f, 0.0f, 0.0f,  0.0f, 2.0f, 0.0f,
    // Z轴
    0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 2.0f
};

// 创建着色器
GLuint createShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, nullptr, info);
        std::cerr << "Shader compile error:\n" << info << std::endl;
    }
    return shader;
}

// 生成MVP矩阵（简单实现，右手坐标系）
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
glm::mat4 getMVP(int width, int height) {
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(3, 3, 3), // 相机位置
        glm::vec3(0, 0, 0), // 目标
        glm::vec3(0, 1, 0)  // 上向量
    );
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), width / (float)height, 0.1f, 100.0f);
    return proj * view * model;
}

int main() {
    // 初始化GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(600, 600, "现代OpenGL三维坐标系", nullptr, nullptr);
    if (!window) {
        std::cerr << "Create window failed\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed\n";
        return -1;
    }

    // 编译着色器
    GLuint vShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);
    glDeleteShader(vShader);
    glDeleteShader(fShader);

    // VAO/VBO
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesVertices), axesVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glm::mat4 mvp = getMVP(width, height);
        GLint mvpLoc = glGetUniformLocation(program, "MVP");
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);

        glBindVertexArray(VAO);

        // X轴 红色
        glUniform3f(glGetUniformLocation(program, "color"), 1.0f, 0.0f, 0.0f);
        glDrawArrays(GL_LINES, 0, 2);
        // Y轴 绿色
        glUniform3f(glGetUniformLocation(program, "color"), 0.0f, 1.0f, 0.0f);
        glDrawArrays(GL_LINES, 2, 2);
        // Z轴 蓝色
        glUniform3f(glGetUniformLocation(program, "color"), 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_LINES, 4, 2);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}