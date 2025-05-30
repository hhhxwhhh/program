#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

using namespace std;

// 🎨 顶点着色器 - 支持MVP矩阵
const char* vertexShaderSource = R"(
#version 430 core
layout (location = 0) in vec3 aPos;

out vec3 vertexColor;

uniform mat4 model;       // 模型矩阵 (旋转、缩放、平移)
uniform mat4 view;        // 视图矩阵 (相机)
uniform mat4 projection;  // 投影矩阵 (透视/正交)
uniform float time;

void main()
{
    // 🎯 MVP变换
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // 基于位置和时间计算颜色
    float r = 0.5 + 0.5 * sin(time + aPos.x * 3.14159);
    float g = 0.5 + 0.5 * sin(time + aPos.y * 3.14159 + 2.094);
    float b = 0.5 + 0.5 * sin(time + aPos.z * 3.14159 + 4.188);
    
    vertexColor = vec3(r, g, b);
}
)";

// 🎨 片段着色器
const char* fragmentShaderSource = R"(
#version 430 core
in vec3 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
)";

// 🛠️ 创建着色器程序
unsigned int createShaderProgram() {
    // 编译顶点着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    // 检查编译错误
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cerr << "顶点着色器编译错误: " << infoLog << endl;
    }
    
    // 编译片段着色器
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cerr << "片段着色器编译错误: " << infoLog << endl;
    }
    
    // 链接着色器程序
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cerr << "着色器程序链接错误: " << infoLog << endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

int main()
{
    // 🚀 初始化GLFW
    if (!glfwInit()) {
        cerr << "GLFW初始化失败" << endl;
        return -1;
    }
    
    // 设置OpenGL版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "GLM旋转示例", NULL, NULL);
    if (!window) {
        cerr << "窗口创建失败" << endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // 初始化GLEW
    if (glewInit() != GLEW_OK) {
        cerr << "GLEW初始化失败" << endl;
        return -1;
    }
    
    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    
    // 创建着色器程序
    unsigned int shaderProgram = createShaderProgram();
    
    // 🔺 定义立方体顶点 (更容易看出旋转效果)
    float vertices[] = {
        // 前面
        -0.5f, -0.5f,  0.5f,  // 0
         0.5f, -0.5f,  0.5f,  // 1
         0.5f,  0.5f,  0.5f,  // 2
        -0.5f,  0.5f,  0.5f,  // 3
        // 后面
        -0.5f, -0.5f, -0.5f,  // 4
         0.5f, -0.5f, -0.5f,  // 5
         0.5f,  0.5f, -0.5f,  // 6
        -0.5f,  0.5f, -0.5f   // 7
    };
    
    unsigned int indices[] = {
        // 前面
        0, 1, 2,   2, 3, 0,
        // 后面
        4, 5, 6,   6, 7, 4,
        // 左面
        7, 3, 0,   0, 4, 7,
        // 右面
        1, 5, 6,   6, 2, 1,
        // 上面
        3, 2, 6,   6, 7, 3,
        // 下面
        0, 1, 5,   5, 4, 0
    };
    
    // 📦 创建VAO, VBO, EBO
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // 设置顶点属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 🎯 获取uniform位置
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
    unsigned int timeLoc = glGetUniformLocation(shaderProgram, "time");
    
    cout << "🎮 GLM旋转演示程序" << endl;
    cout << "功能展示:" << endl;
    cout << "  - 绕X轴旋转 (红色轴)" << endl;
    cout << "  - 绕Y轴旋转 (绿色轴)" << endl;
    cout << "  - 绕Z轴旋转 (蓝色轴)" << endl;
    cout << "  - 组合旋转效果" << endl;
    cout << "按ESC退出" << endl;
    
    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        // 处理输入
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        // 清除缓冲区
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 获取时间
        float currentTime = static_cast<float>(glfwGetTime());
        
        // 🚀 使用GLM创建各种变换矩阵
        
        // 1️⃣ 模型矩阵 (物体的变换)
        glm::mat4 model = glm::mat4(1.0f);  // 单位矩阵
        
        // 🔄 组合旋转 - 这是GLM的核心功能！
        model = glm::rotate(model, currentTime * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));  // 绕X轴
        model = glm::rotate(model, currentTime * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f));  // 绕Y轴
        model = glm::rotate(model, currentTime * 0.7f, glm::vec3(0.0f, 0.0f, 1.0f));  // 绕Z轴
        
        // 🔍 可选：添加其他变换
        // model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));  // 平移
        // model = glm::scale(model, glm::vec3(1.0f + 0.3f * sin(currentTime), 1.0f, 1.0f));  // 缩放
        
        // 2️⃣ 视图矩阵 (相机位置)
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));  // 相机后退3单位
        
        // 🎥 可选：环绕相机
        // float radius = 3.0f;
        // float camX = sin(currentTime * 0.5f) * radius;
        // float camZ = cos(currentTime * 0.5f) * radius;
        // view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), 
        //                    glm::vec3(0.0f, 0.0f, 0.0f), 
        //                    glm::vec3(0.0f, 1.0f, 0.0f));
        
        // 3️⃣ 投影矩阵 (透视投影)
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),                    // FOV
            800.0f / 600.0f,                        // 宽高比
            0.1f,                                   // 近平面
            100.0f                                  // 远平面
        );
        
        // 🎯 使用着色器程序
        glUseProgram(shaderProgram);
        
        // 📤 传递矩阵到着色器
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(timeLoc, currentTime);
        
        // 🎨 绘制立方体
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // 交换缓冲区
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // 🧹 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    
    glfwTerminate();
    
    cout << "程序结束！" << endl;
    return 0;
}