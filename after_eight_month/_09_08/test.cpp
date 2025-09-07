#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

// 顶点着色器 - 传递控制点
const char* vertexShaderSource = R"(
#version 430 core
layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
}
)";

// 细分控制着色器 - 控制细分级别
const char* tessControlShaderSource = R"(
#version 430 core
layout (vertices = 16) out;

uniform float tessellationLevel;

void main()
{
    // 传递控制点位置
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    
    // 设置细分级别
    if (gl_InvocationID == 0) {
        // 内部细分级别
        gl_TessLevelInner[0] = tessellationLevel;
        gl_TessLevelInner[1] = tessellationLevel;
        
        // 外部细分级别
        gl_TessLevelOuter[0] = tessellationLevel;
        gl_TessLevelOuter[1] = tessellationLevel;
        gl_TessLevelOuter[2] = tessellationLevel;
        gl_TessLevelOuter[3] = tessellationLevel;
    }
}
)";

// 细分评估着色器 - 计算细分后的顶点位置
const char* tessEvaluationShaderSource = R"(
#version 430 core
layout (quads, equal_spacing, ccw) in;

// 贝塞尔曲面的双三次基函数
float bernstein(float u, int i) {
    switch(i) {
        case 0: return (1.0 - u) * (1.0 - u) * (1.0 - u);
        case 1: return 3.0 * u * (1.0 - u) * (1.0 - u);
        case 2: return 3.0 * u * u * (1.0 - u);
        case 3: return u * u * u;
        default: return 0.0;
    }
}

vec3 bezierPatch(vec3 cp[16], vec2 uv) {
    vec3 position = vec3(0.0);
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float Bi = bernstein(uv.x, i);
            float Bj = bernstein(uv.y, j);
            position += cp[i * 4 + j] * Bi * Bj;
        }
    }
    
    return position;
}

// 接收从顶点着色器传递过来的矩阵
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // 获取细分坐标
    vec2 texCoord = gl_TessCoord.xy;
    
    // 从控制点数组重建控制点
    vec3 controlPoints[16];
    for (int i = 0; i < 16; i++) {
        controlPoints[i] = gl_in[i].gl_Position.xyz;
    }
    
    // 计算贝塞尔曲面上的点
    vec3 position = bezierPatch(controlPoints, texCoord);
    
    // 使用传递进来的矩阵而不是内置的gl_ModelViewProjectionMatrix
    gl_Position = projection * view * model * vec4(position, 1.0);
}
)";

// 片段着色器 - 计算光照
const char* fragmentShaderSource = R"(
#version 430 core
out vec4 FragColor;

// 从细分评估着色器接收法线和位置
in vec3 Normal;
in vec3 FragPos;
in vec3 WorldPos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // 环境光
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    // 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // 镜面反射
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
)";

// 函数声明
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
unsigned int compileShader(unsigned int type, const char* source);
unsigned int createShaderProgram();
vector<glm::vec3> generateBezierPatchControlPoints();

// 设置
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// 相机
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

// 计时
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 细分级别
float tessellationLevel = 10.0f;

int main()
{
    // 初始化和配置
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Bezier Surface Tessellation", NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 初始化GLEW
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize GLEW" << endl;
        return -1;
    }

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);

    // 构建和编译着色器程序
    unsigned int shaderProgram = createShaderProgram();

    // 生成贝塞尔曲面控制点
    vector<glm::vec3> controlPoints = generateBezierPatchControlPoints();
    
    // 创建VAO和VBO
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(glm::vec3), &controlPoints[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    // 获取uniform位置
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    unsigned int tessLevelLoc = glGetUniformLocation(shaderProgram, "tessellationLevel");
    unsigned int lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    unsigned int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    unsigned int lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    unsigned int objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 计算帧时间差
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 输入处理
        processInput(window);

        // 渲染
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 激活着色器
        glUseProgram(shaderProgram);

        // 传递变换矩阵
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime() * 0.2f, glm::vec3(0.0f, 1.0f, 0.0f)); // 自动旋转
        
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // 传递光照参数
        glUniform3f(lightPosLoc, 2.0f, 2.0f, 3.0f);
        glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
        glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
        glUniform3f(objectColorLoc, 0.8f, 0.3f, 0.2f);

        // 传递细分级别
        glUniform1f(tessLevelLoc, tessellationLevel);

        // 绘制贝塞尔曲面
        glBindVertexArray(VAO);
        glPatchParameteri(GL_PATCH_VERTICES, 16); // 每个面片有16个控制点
        glDrawArrays(GL_PATCHES, 0, 16); // 绘制16个控制点定义的面片

        // 交换缓冲区和轮询IO事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

// 处理所有输入：查询GLFW是否按下相关按键
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    
    // 调整细分级别
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        tessellationLevel = min(tessellationLevel + 1.0f, 64.0f);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        tessellationLevel = max(tessellationLevel - 1.0f, 1.0f);
}

// 当窗口大小改变时，调用这个回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// 编译着色器
unsigned int compileShader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    // 检查编译错误
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << endl;
    }
    
    return shader;
}

// 创建着色器程序
unsigned int createShaderProgram()
{
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int tessControlShader = compileShader(GL_TESS_CONTROL_SHADER, tessControlShaderSource);
    unsigned int tessEvaluationShader = compileShader(GL_TESS_EVALUATION_SHADER, tessEvaluationShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, tessControlShader);
    glAttachShader(shaderProgram, tessEvaluationShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // 检查链接错误
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(tessControlShader);
    glDeleteShader(tessEvaluationShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

// 生成贝塞尔曲面控制点（一个波浪形曲面）
vector<glm::vec3> generateBezierPatchControlPoints()
{
    vector<glm::vec3> controlPoints(16);
    
    // 定义4x4控制点网格，创建一个波浪形曲面
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float x = (float)(j - 1.5f);
            float y = (float)(i - 1.5f);
            // 创建波浪效果
            float z = 0.3f * sin(x * 2.0f) * cos(y * 2.0f);
            controlPoints[i * 4 + j] = glm::vec3(x, y, z);
        }
    }
    
    return controlPoints;
}