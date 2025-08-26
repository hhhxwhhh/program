#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <cmath>

// 顶点着色器源码
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aPos;
}
)";

// 片段着色器源码 - 冰效果
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D ourTexture;
uniform float time;

void main()
{
    // 基础纹理颜色
    vec4 texColor = texture(ourTexture, TexCoord);
    
    // 计算法线
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    
    // 简单的光照计算
    float diff = max(dot(norm, lightDir), 0.2);
    vec3 diffuse = diff * vec3(0.8, 0.9, 1.0);
    
    // 冰的折射效果
    vec2 distortion = vec2(sin(TexCoord.x * 10.0 + time), cos(TexCoord.y * 10.0 + time)) * 0.01;
    vec4 iceTex = texture(ourTexture, TexCoord + distortion);
    
    // 冰的透明和反射效果
    float alpha = 0.7 + 0.3 * sin(time * 2.0);
    vec3 iceColor = mix(iceTex.rgb, vec3(0.7, 0.9, 1.0), 0.3);
    
    // 最终颜色
    vec3 result = iceColor * diffuse;
    FragColor = vec4(result, alpha);
}
)";

// 错误检查函数
void checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << std::endl;
        }
    }
}

// 创建冰纹理
unsigned int createIceTexture()
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // 创建冰纹理数据 - 蓝白色噪点
    const int width = 8;
    const int height = 8;
    unsigned char data[8*8*3];
    
    for (int i = 0; i < width * height; i++) {
        // 创建冰的蓝白色调
        unsigned char blue = 200 + rand() % 55;
        unsigned char green = 220 + rand() % 35;
        unsigned char red = 200 + rand() % 55;
        
        data[i*3] = red;
        data[i*3 + 1] = green;
        data[i*3 + 2] = blue;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // 设置纹理环绕和过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    return texture;
}

int main()
{
    // 初始化GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "Ice Pyramid", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 初始化GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // 启用深度测试和混合
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 定义金字塔顶点数据（位置+纹理坐标）
    float vertices[] = {
        // 底面
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        // 前面
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.0f,  0.5f,  0.0f,  0.5f, 1.0f,

        // 右面
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.0f,  0.5f,  0.0f,  0.5f, 1.0f,

        // 后面
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.0f,  0.5f,  0.0f,  0.5f, 1.0f,

        // 左面
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.0f,  0.5f,  0.0f,  0.5f, 1.0f
    };

    // 创建VAO, VBO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 设置顶点属性指针
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 纹理坐标属性
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 编译着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    // 链接着色器程序
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkCompileErrors(shaderProgram, "PROGRAM");

    // 删除着色器对象
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 加载纹理
    unsigned int texture = createIceTexture();
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture"), 0);

    // 获取uniform位置
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
    unsigned int timeLoc = glGetUniformLocation(shaderProgram, "time");

    // 设置投影矩阵（透视投影）
    float fov = 45.0f;
    float aspect = 800.0f / 600.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    
    // 修复类型转换警告
    float fovRad = fov * 3.14159f / 180.0f;
    float tanFov = tanf(fovRad/2.0f);
    
    float projMatrix[16] = {
        1.0f/(aspect*tanFov), 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f/tanFov, 0.0f, 0.0f,
        0.0f, 0.0f, -(farPlane+nearPlane)/(farPlane-nearPlane), -1.0f,
        0.0f, 0.0f, -(2.0f*farPlane*nearPlane)/(farPlane-nearPlane), 0.0f
    };
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projMatrix);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 处理输入
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // 清除屏幕
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 激活并绑定纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        // 使用着色器程序
        glUseProgram(shaderProgram);

        // 传递时间uniform
        float currentTime = glfwGetTime();
        glUniform1f(timeLoc, currentTime);

        // 设置视图矩阵（摄像机位置）
        float viewMatrix[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, -2.5f, 1.0f
        };
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMatrix);

        // 设置模型矩阵（旋转和平移）
        float timeValue = glfwGetTime();
        float angle = timeValue * 20.0f; // 每秒旋转20度
        
        // 修复类型转换警告
        float angleRad = angle * 3.14159f / 180.0f;
        float cosAngle = cosf(angleRad);
        float sinAngle = sinf(angleRad);
        
        // 旋转矩阵（绕Y轴旋转）
        float modelMatrix[16] = {
            cosAngle, 0.0f, sinAngle, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            -sinAngle, 0.0f, cosAngle, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelMatrix);

        // 绘制金字塔
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 18);

        // 交换缓冲区和轮询IO事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &texture);

    glfwTerminate();
    return 0;
}