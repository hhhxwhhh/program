#include <iostream>
#include <vector>
#include <random>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

// 🎯 立方体数量
const int CUBE_COUNT = 10000;

// 🎨 实例化顶点着色器
const char* vertexShaderSource = R"(
#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aInstancePos;    // 🆕 实例位置
layout (location = 2) in vec3 aInstanceColor;  // 🆕 实例颜色
layout (location = 3) in float aInstanceScale; // 🆕 实例缩放
layout (location = 4) in float aInstanceSpeed; // 🆕 实例动画速度

out vec3 vertexColor;

uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    // 🔄 基于时间和实例速度的动态旋转
    float rotationAngle = time * aInstanceSpeed;
    
    // 创建旋转矩阵
    mat4 rotation = mat4(1.0);
    rotation[0][0] = cos(rotationAngle);
    rotation[0][2] = sin(rotationAngle);
    rotation[2][0] = -sin(rotationAngle);
    rotation[2][2] = cos(rotationAngle);
    
    // 应用缩放
    vec3 scaledPos = aPos * aInstanceScale;
    
    // 应用旋转
    vec4 rotatedPos = rotation * vec4(scaledPos, 1.0);
    
    // 应用位置偏移
    vec4 worldPos = vec4(rotatedPos.xyz + aInstancePos, 1.0);
    
    // 🎯 最终变换
    gl_Position = projection * view * worldPos;
    
    // 🌈 基于实例颜色和时间的动态颜色
    float colorIntensity = 0.7 + 0.3 * sin(time * aInstanceSpeed + length(aInstancePos));
    vertexColor = aInstanceColor * colorIntensity;
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

// 🛠️ 着色器编译函数
unsigned int compileShader(const char* source, GLenum type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cout << "❌ Shader compilation error: " << infoLog << endl;
        return 0;
    }
    return shader;
}

// 🛠️ 创建着色器程序
unsigned int createShaderProgram() {
    unsigned int vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        return 0;
    }
    
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        cout << "❌ Program linking error: " << infoLog << endl;
        return 0;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    cout << "✅ Shader program created successfully!" << endl;
    return program;
}

// 🎮 键盘处理
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// 📏 窗口大小回调
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main()
{
    cout << "🚀 10,000 Cubes Instance Rendering Demo!" << endl;
    cout << "准备创建 " << CUBE_COUNT << " 个动画立方体..." << endl;
    
    // 🎬 初始化GLFW
    if (!glfwInit()) {
        cout << "❌ GLFW initialization failed!" << endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(1200, 900, "🎲 10,000 动画立方体", NULL, NULL);
    if (!window) {
        cout << "❌ Window creation failed!" << endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // 初始化GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        cout << "❌ GLEW initialization failed!" << endl;
        return -1;
    }
    
    cout << "✅ OpenGL " << glGetString(GL_VERSION) << endl;
    
    // 启用深度测试和面剔除
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // 创建着色器程序
    unsigned int shaderProgram = createShaderProgram();
    if (shaderProgram == 0) {
        cout << "❌ Failed to create shader program!" << endl;
        return -1;
    }
    
    // 📦 立方体顶点数据 (只需要一个立方体的数据)
    float vertices[] = {
        // 前面
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        // 后面
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f
    };
    
    unsigned int indices[] = {
        0, 1, 2,   2, 3, 0,   // 前面
        4, 5, 6,   6, 7, 4,   // 后面
        7, 3, 0,   0, 4, 7,   // 左面
        1, 5, 6,   6, 2, 1,   // 右面
        3, 2, 6,   6, 7, 3,   // 上面
        0, 1, 5,   5, 4, 0    // 下面
    };
    
    // 🎲 生成实例数据
    cout << "🎨 生成 " << CUBE_COUNT << " 个立方体的实例数据..." << endl;
    
    vector<vec3> instancePositions(CUBE_COUNT);
    vector<vec3> instanceColors(CUBE_COUNT);
    vector<float> instanceScales(CUBE_COUNT);
    vector<float> instanceSpeeds(CUBE_COUNT);
    
    // 🎰 随机数生成器
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> posRange(-50.0f, 50.0f);  // 位置范围
    uniform_real_distribution<float> colorRange(0.3f, 1.0f);   // 颜色范围
    uniform_real_distribution<float> scaleRange(0.2f, 1.5f);   // 缩放范围
    uniform_real_distribution<float> speedRange(0.5f, 3.0f);   // 速度范围
    
    // 🌈 生成每个立方体的属性
    for (int i = 0; i < CUBE_COUNT; i++) {
        // 随机位置
        instancePositions[i] = vec3(
            posRange(gen),
            posRange(gen),
            posRange(gen)
        );
        
        // 随机颜色
        instanceColors[i] = vec3(
            colorRange(gen),
            colorRange(gen),
            colorRange(gen)
        );
        
        // 随机缩放
        instanceScales[i] = scaleRange(gen);
        
        // 随机旋转速度
        instanceSpeeds[i] = speedRange(gen);
    }
    
    cout << "✅ 实例数据生成完成!" << endl;
    
    // 📦 创建VAO, VBO, EBO
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    // 🔺 基础几何数据
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // 设置顶点位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 🆕 创建实例缓冲区
    unsigned int instanceVBO[4];  // 位置、颜色、缩放、速度
    glGenBuffers(4, instanceVBO);
    
    // 🎯 实例位置缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * CUBE_COUNT, &instancePositions[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);  // 🔑 每个实例使用一次这个属性
    
    // 🌈 实例颜色缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * CUBE_COUNT, &instanceColors[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);  // 🔑 每个实例使用一次这个属性
    
    // 📏 实例缩放缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * CUBE_COUNT, &instanceScales[0], GL_STATIC_DRAW);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);  // 🔑 每个实例使用一次这个属性
    
    // ⚡ 实例速度缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * CUBE_COUNT, &instanceSpeeds[0], GL_STATIC_DRAW);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);  // 🔑 每个实例使用一次这个属性
    
    cout << "📦 VAO和实例缓冲区设置完成!" << endl;
    
    // 🎯 获取uniform位置
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint timeLoc = glGetUniformLocation(shaderProgram, "time");
    
    // 验证uniform
    if (viewLoc == -1) cout << "⚠️ Warning: 'view' uniform not found!" << endl;
    if (projLoc == -1) cout << "⚠️ Warning: 'projection' uniform not found!" << endl;
    if (timeLoc == -1) cout << "⚠️ Warning: 'time' uniform not found!" << endl;
    
    cout << "✅ Uniform locations: view=" << viewLoc << ", proj=" << projLoc << ", time=" << timeLoc << endl;
    
    // 📱 相机参数
    vec3 cameraPos = vec3(0.0f, 0.0f, 100.0f);
    vec3 cameraTarget = vec3(0.0f, 0.0f, 0.0f);
    vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);
    
    // 📋 程序说明
    cout << "\n🎮 10,000 动画立方体演示" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "🎯 特性:" << endl;
    cout << "   ✨ " << CUBE_COUNT << " 个独立动画的立方体" << endl;
    cout << "   🌈 每个立方体有随机颜色" << endl;
    cout << "   📏 每个立方体有随机大小" << endl;
    cout << "   ⚡ 每个立方体有独立旋转速度" << endl;
    cout << "   🎨 动态颜色变化" << endl;
    cout << "   🚀 GPU实例化渲染技术" << endl;
    cout << "\n🎮 控制:" << endl;
    cout << "   ESC - 退出程序" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;
    
    cout << "🎬 开始渲染 " << CUBE_COUNT << " 个立方体..." << endl;
    
    // 🎬 主渲染循环
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        
        float currentTime = static_cast<float>(glfwGetTime());
        
        // 🎨 设置背景色 (深空色)
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 📷 创建视图矩阵 (环绕相机)
        float radius = 80.0f;
        float camX = sin(currentTime * 0.2f) * radius;
        float camZ = cos(currentTime * 0.2f) * radius;
        float camY = sin(currentTime * 0.1f) * 20.0f;
        
        mat4 view = lookAt(
            vec3(camX, camY, camZ),      // 相机位置
            vec3(0.0f, 0.0f, 0.0f),      // 看向中心
            vec3(0.0f, 1.0f, 0.0f)       // 上向量
        );
        
        // 🎭 创建投影矩阵
        mat4 projection = perspective(
            radians(45.0f),              // FOV
            1200.0f / 900.0f,           // 宽高比
            0.1f,                       // 近平面
            200.0f                      // 远平面
        );
        
        // 🎯 使用着色器程序
        glUseProgram(shaderProgram);
        
        // 📤 传递uniform
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projection));
        glUniform1f(timeLoc, currentTime);
        
        // 🚀 实例化渲染 - 一次调用绘制所有立方体！
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, CUBE_COUNT);
        
        // 🔄 交换缓冲区
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // 🧹 清理资源
    cout << "\n🧹 清理资源..." << endl;
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(4, instanceVBO);
    glDeleteProgram(shaderProgram);
    
    glfwTerminate();
    
    cout << "✅ 10,000 立方体演示结束!" << endl;
    return 0;
}