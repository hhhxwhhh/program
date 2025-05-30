#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <cmath>

using namespace std;
using namespace glm;

// 修复的顶点着色器
const char *vertexShaderSource = R"(
#version 430 core
layout (location = 0) in vec3 aPos;

out vec3 vertexColor;

uniform mat4 model;        
uniform mat4 view;         
uniform mat4 projection;   
uniform float time;        

void main()
{
    // 使用正确的变量名
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // 基于位置和时间计算颜色
    float r = 0.5 + 0.5 * sin(time + aPos.x * 3.14159);
    float g = 0.5 + 0.5 * sin(time + aPos.y * 3.14159 + 2.094);
    float b = 0.5 + 0.5 * sin(time + aPos.z * 3.14159 + 4.188);
    
    vertexColor = vec3(r, g, b);
}
)";

// 片段着色器 (无需修改)
const char *fragmentShaderSource = R"(
#version 430 core
in vec3 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
)";

// 创建着色器程序 (无需修改)
unsigned int createShaderProgram()
{
    // 编译顶点着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    int success;
    char infolog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infolog);
        cout << " ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infolog << endl;
        return 0;
    }
    
    // 编译片段着色器
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infolog);
        cout << " ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infolog << endl;
        return 0;
    }
    
    // 创建着色器程序
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infolog);
        cout << " ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infolog << endl;
        return 0;
    }
    
    // 删除着色器对象
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    cout << " Shader program created successfully!" << endl;
    return shaderProgram;
}

// 键盘回调函数
void processInput(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

// 窗口大小改变回调
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    cout << " Hello OpenGL - First Cube Demo!" << endl;
    
    // 初始化GLFW
    if(!glfwInit())
    {
        cout << " GLFW initialization failed!" << endl;
        return -1;
    }
    
    // 设置OpenGL版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "🎲 First Cube", NULL, NULL);
    if(!window)
    {
        cout << " Window creation failed!" << endl;
        glfwTerminate();
        return -1;
    }
    
    // 设置上下文
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 初始化GLEW
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
    {
        cout << " GLEW init failed!" << endl;
        return -1;
    }

    // 打印OpenGL信息
    cout << " OpenGL Version: " << glGetString(GL_VERSION) << endl;
    cout << " GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);

    // 创建着色器程序
    unsigned int shaderProgram = createShaderProgram();
    if(shaderProgram == 0) {
        cout << " Failed to create shader program!" << endl;
        return -1;
    }

    // 定义立方体的顶点数据
    float vertices[] = {
        // 前面
        -0.5f, -0.5f,  0.5f, // 0
         0.5f, -0.5f,  0.5f, // 1
         0.5f,  0.5f,  0.5f, // 2
        -0.5f,  0.5f,  0.5f, // 3
        // 后面
        -0.5f, -0.5f, -0.5f, // 4
         0.5f, -0.5f, -0.5f, // 5
         0.5f,  0.5f, -0.5f, // 6
        -0.5f,  0.5f, -0.5f  // 7
    };

    // 定义立方体的索引数据
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

    // 创建VAO、VBO和EBO
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    cout << "✅ VAO, VBO, EBO created successfully!" << endl;

    // 绑定VAO
    glBindVertexArray(VAO);

    // 绑定VBO并传递顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 绑定EBO并传递索引数据
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ✅ 获取uniform位置 (使用正确的变量名)
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    unsigned int timeLoc = glGetUniformLocation(shaderProgram, "time");

    // 验证uniform位置
    if(modelLoc == -1) cout << " Warning: 'model' uniform not found!" << endl;
    if(viewLoc == -1) cout << " Warning: 'view' uniform not found!" << endl;
    if(projectionLoc == -1) cout << " Warning: 'projection' uniform not found!" << endl;
    if(timeLoc == -1) cout << " Warning: 'time' uniform not found!" << endl;

    cout << "✅ Uniform locations retrieved successfully!" << endl;
    cout << "   Model: " << modelLoc << ", View: " << viewLoc 
         << ", Projection: " << projectionLoc << ", Time: " << timeLoc << endl;

    // 程序说明
    cout << "\n🎮 OpenGL Cube Rotation Demo" << endl;
    cout << "Controls:" << endl;
    cout << "  - Press ESC to exit" << endl;
    cout << "  - Observe the cube rotating around X, Y, and Z axes" << endl;
    cout << "Starting render loop...\n" << endl;

    // 主渲染循环
    while(!glfwWindowShouldClose(window))
    {
        srand(static_cast<unsigned int>(glfwGetTime())); // 随机种子
        // 处理输入
        processInput(window);

        // 获取时间 
        float currentTime = static_cast<float>(glfwGetTime());

        // 设置背景颜色和清除缓冲区
        glClearColor(static_cast<float>(rand())/ RAND_MAX, static_cast<float>(rand())/ RAND_MAX, static_cast<float>(rand())/ RAND_MAX, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 创建变换矩阵
        mat4 model = mat4(1.0f);
        model = rotate(model, currentTime * 0.5f, vec3(1.0f, 0.0f, 0.0f)); // 绕X轴旋转
        model = rotate(model, currentTime * 0.7f, vec3(0.0f, 1.0f, 0.0f)); // 绕Y轴旋转
        model = rotate(model, currentTime * 0.9f, vec3(0.0f, 0.0f, 1.0f)); // 绕Z轴旋转

        // 呼吸效果
        float breathe = 1.0f + 0.1f * sin(currentTime);
        model = scale(model, vec3(breathe, breathe, breathe));

        // 视图矩阵
        mat4 view = lookAt(
            vec3(0.0f, 0.0f, 3.0f),  // 相机位置
            vec3(0.0f, 0.0f, 0.0f),  // 目标点
            vec3(0.0f, 1.0f, 0.0f)   // 上向量
        );

        // 投影矩阵
        mat4 projection = perspective(
            radians(45.0f),      // FOV
            800.0f / 600.0f,     // 宽高比
            0.1f,                // 近平面
            100.0f               // 远平面
        );

        // 使用着色器程序
        glUseProgram(shaderProgram);

        // 正确传递uniform变量
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, value_ptr(projection));
        glUniform1f(timeLoc, currentTime);

        // 绘制立方体
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // 交换缓冲区
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();

    cout << "OpenGL Cube Demo finished successfully!" << endl;
    return 0;
}