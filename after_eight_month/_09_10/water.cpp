#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

// 窗口尺寸
const GLuint WIDTH = 800, HEIGHT = 600;

// 顶点着色器
const GLchar* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 ClipSpace;
out vec3 ViewDir;
out vec3 LightDir;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform vec3 viewPos;
uniform vec3 lightPos;

void main()
{
    // Gerstner波参数
    float frequencies[4];
    frequencies[0] = 0.8;
    frequencies[1] = 1.2;
    frequencies[2] = 1.7;
    frequencies[3] = 2.1;
    
    float amplitudes[4];
    amplitudes[0] = 0.15;
    amplitudes[1] = 0.1;
    amplitudes[2] = 0.05;
    amplitudes[3] = 0.03;
    
    float speeds[4];
    speeds[0] = 0.5;
    speeds[1] = 1.0;
    speeds[2] = 1.3;
    speeds[3] = 1.8;
    
    vec2 directions[4];
    directions[0] = normalize(vec2(1.0, 0.0));
    directions[1] = normalize(vec2(0.0, 1.0));
    directions[2] = normalize(vec2(0.7, 0.7));
    directions[3] = normalize(vec2(-0.7, 0.7));

    vec3 worldPosition = position;
    vec3 tangent = vec3(1.0, 0.0, 0.0);
    vec3 bitangent = vec3(0.0, 0.0, 1.0);
    
    // 计算Gerstner波
    float waveHeight = 0.0;
    vec2 waveDerivativeX = vec2(0.0);
    vec2 waveDerivativeZ = vec2(0.0);
    
    for(int i = 0; i < 4; i++) {
        float frequency = frequencies[i];
        float amplitude = amplitudes[i];
        float speed = speeds[i];
        vec2 direction = directions[i];
        
        float dotProd = dot(position.xz, direction);
        float waveParam = dotProd * frequency + time * speed;
        
        // Gerstner波公式
        waveHeight += amplitude * sin(waveParam);
        
        // 计算偏导数用于法线计算
        float Q = 0.8 / frequency; // 陡峭度参数
        waveDerivativeX += vec2(
            Q * amplitude * direction.x * cos(waveParam),
            amplitude * direction.x * cos(waveParam)
        );
        waveDerivativeZ += vec2(
            Q * amplitude * direction.y * cos(waveParam),
            amplitude * direction.y * cos(waveParam)
        );
    }
    
    // 更新顶点位置
    worldPosition.y = waveHeight;
    worldPosition.x += waveDerivativeX.x;
    worldPosition.z += waveDerivativeZ.x;
    
    // 计算法线
    vec3 dx = vec3(
        1.0 - waveDerivativeX.y,
        -waveDerivativeX.y,
        -waveDerivativeZ.y
    );
    vec3 dz = vec3(
        -waveDerivativeX.y,
        -waveDerivativeZ.y,
        1.0 - waveDerivativeZ.y
    );
    vec3 perturbatedNormal = normalize(cross(dx, dz));
    
    ClipSpace = projection * view * model * vec4(worldPosition, 1.0);
    FragPos = vec3(model * vec4(worldPosition, 1.0));
    Normal = mat3(transpose(inverse(model))) * perturbatedNormal;
    TexCoords = texCoords * 5.0; // 放大纹理坐标以重复纹理
    
    // 计算光照和视图方向
    ViewDir = viewPos - FragPos;
    LightDir = lightPos - FragPos;
    
    gl_Position = ClipSpace;
}
)";

// 片段着色器
const GLchar* fragmentShaderSource = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 ClipSpace;
in vec3 ViewDir;
in vec3 LightDir;

out vec4 color;

uniform float time;
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

// 模拟水面纹理效果
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f*f*(3.0-2.0*f);

    return mix(a, b, u.x) + 
            (c - a)* u.y * (1.0 - u.x) + 
            (d - b) * u.x * u.y;
}

float fbm(vec2 st) {
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 4; i++) {
        value += amplitude * noise(st);
        st *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

void main()
{
    // 基础水面颜色
    vec3 waterColor = vec3(0.0, 0.3, 0.6);
    vec3 deepColor = vec3(0.0, 0.1, 0.3);
    vec3 shallowColor = vec3(0.0, 0.5, 0.8);
    
    // 计算法向量
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(ViewDir);
    vec3 lightDir = normalize(LightDir);
    vec3 reflectDir = reflect(-lightDir, norm);
    
    // 模拟菲涅尔效应
    float fresnel = dot(viewDir, norm);
    fresnel = pow(1.0 - fresnel, 3.0);
    fresnel = clamp(fresnel, 0.0, 1.0);
    
    // 漫反射
    float diff = max(dot(norm, lightDir), 0.0);
    
    // 镜面反射
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128.0);
    
    // 泡沫效果 - 基于顶点法线的Y值
    float foam = 1.0 - abs(Normal.y);
    foam = smoothstep(0.8, 1.0, foam);
    
    // 基于深度的颜色混合
    float depthFactor = clamp(length(ViewDir) / 20.0, 0.0, 1.0);
    vec3 depthColor = mix(shallowColor, deepColor, depthFactor);
    
    // 添加噪声纹理效果
    vec2 noiseCoords = TexCoords + vec2(time * 0.02, time * 0.01);
    float noiseValue = fbm(noiseCoords);
    
    // 最终颜色计算
    vec3 ambient = 0.1 * waterColor;
    vec3 diffuse = diff * lightColor * waterColor;
    vec3 specular = spec * lightColor * 2.0;
    
    // 组合所有效果
    vec3 result = ambient + diffuse + specular;
    result = mix(result, vec3(1.0), foam * 0.7); // 添加泡沫
    result = mix(result, depthColor, 0.5); // 添加深度效果
    
    // 基于噪声调整颜色
    result *= (0.9 + 0.1 * noiseValue);
    
    // 透明度
    float alpha = 0.8 - foam * 0.3;
    alpha = mix(alpha, 0.95, foam); // 泡沫区域更不透明
    
    color = vec4(result, alpha);
}
)";

// 生成平面网格
vector<float> generatePlaneVertices(float width, float height, int subdivisions) 
{
    vector<float> vertices;
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;
    float stepX = width / subdivisions;
    float stepZ = height / subdivisions;
    
    for (int i = 0; i <= subdivisions; i++) 
    {
        for (int j = 0; j <= subdivisions; j++) 
        {
            float x = -halfWidth + j * stepX;
            float z = -halfHeight + i * stepZ;
            float y = 0.0f;
            
            // 位置
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            
            // 法线
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
            
            // 纹理坐标
            vertices.push_back((float)j / subdivisions);
            vertices.push_back((float)i / subdivisions);
        }
    }
    return vertices;
}

// 生成索引
vector<unsigned int> generatePlaneIndices(int subdivisions) 
{
    vector<unsigned int> indices;
    for (int i = 0; i < subdivisions; i++) 
    {
        for (int j = 0; j < subdivisions; j++) 
        {
            int topLeft = i * (subdivisions + 1) + j;
            int topRight = topLeft + 1;
            int bottomLeft = (i + 1) * (subdivisions + 1) + j;
            int bottomRight = bottomLeft + 1;
            
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    return indices;
}

// 编译着色器
GLuint compileShader(GLenum type, const GLchar* source) 
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) 
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << endl;
    }
    
    return shader;
}

// 创建着色器程序
GLuint createShaderProgram() 
{
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) 
    {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

// 处理键盘输入
void processInput(GLFWwindow *window) 
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
    {
        glfwSetWindowShouldClose(window, true);
    }
}

// 主函数
int main() 
{
    // 初始化GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Advanced Water Simulation", nullptr, nullptr);
    if (window == nullptr) 
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // 初始化GLEW
    if (glewInit() != GLEW_OK) 
    {
        cout << "Failed to initialize GLEW" << endl;
        return -1;
    }
    
    // 设置视口
    glViewport(0, 0, WIDTH, HEIGHT);
    
    // 启用深度测试和混合
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 创建着色器程序
    GLuint shaderProgram = createShaderProgram();
    
    // 生成平面网格
    const int subdivisions = 100;
    vector<float> vertices = generatePlaneVertices(30.0f, 30.0f, subdivisions);
    vector<unsigned int> indices = generatePlaneIndices(subdivisions);
    
    // 创建VAO, VBO, EBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 法线属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 纹理坐标属性
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    
    // 设置变换矩阵
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 5.0f, 10.0f), 
                                 glm::vec3(0.0f, 0.0f, 0.0f), 
                                 glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    
    // 光照参数
    glm::vec3 lightPos(10.0f, 10.0f, 10.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 viewPos(0.0f, 5.0f, 10.0f);
    
    // 获取uniform位置
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint timeLoc = glGetUniformLocation(shaderProgram, "time");
    GLuint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    GLuint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    GLuint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    
    // 渲染循环
    while (!glfwWindowShouldClose(window)) 
    {
        // 处理输入
        processInput(window);
        
        // 清除屏幕
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 使用着色器程序
        glUseProgram(shaderProgram);
        
        // 设置变换矩阵
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        
        // 更新时间
        float time = glfwGetTime();
        glUniform1f(timeLoc, time);
        glUniform3f(viewPosLoc, viewPos.x, viewPos.y, viewPos.z);
        glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(lightColorLoc, lightColor.x, lightColor.y, lightColor.z);
        
        // 绘制水面
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        // 交换缓冲区和轮询IO事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    
    glfwTerminate();
    return 0;
}