#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace std;
#define M_PI 3.14159265358979323846

// 全局变量
float triangleRotation = 0.0f;
float triangleScale = 1.0f;
glm::vec3 trianglePosition(0.0f, 0.0f, 0.0f);
glm::vec3 triangleColor(1.0f, 0.5f, 0.2f);
bool autoRotate = false;
bool wireframeMode = false;
int shapeMode = 0; // 0=三角形, 1=正方形, 2=圆形, 3=星形
float animationSpeed = 1.0f;

// 高级顶点着色器 - 支持变换和动画
const char *vertexshadersource = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;

out vec3 vertexColor;

uniform mat4 transform;
uniform mat4 projection;
uniform float time;
uniform float waveEffect;

void main()
{
    vec3 pos = aPos;
    
    // 添加波浪效果
    if(waveEffect > 0.0) {
        pos.y += sin(pos.x * 10.0 + time * 3.0) * 0.1 * waveEffect;
        pos.x += cos(pos.y * 8.0 + time * 2.0) * 0.05 * waveEffect;
    }
    
    gl_Position = projection * transform * vec4(pos, 1.0);
    vertexColor = aColor;
}
)";

// 高级片段着色器 - 支持渐变和特效
const char *fragmentshadersource = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

uniform float time;
uniform vec3 baseColor;
uniform float colorAnimation;
uniform int effectMode;

void main()
{
    vec3 color = vertexColor;
    
    // 效果模式
    if(effectMode == 1) {
        // 彩虹效果
        color = vec3(
            sin(time * 2.0) * 0.5 + 0.5,
            sin(time * 2.0 + 2.094) * 0.5 + 0.5,
            sin(time * 2.0 + 4.188) * 0.5 + 0.5
        );
    } else if(effectMode == 2) {
        // 脉冲效果
        float pulse = sin(time * 4.0) * 0.3 + 0.7;
        color = baseColor * pulse;
    } else if(effectMode == 3) {
        // 渐变效果
        color = mix(baseColor, vec3(1.0, 1.0, 1.0), sin(time) * 0.5 + 0.5);
    }
    
    FragColor = vec4(color, 1.0);
}
)";

// 键盘回调函数
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch(key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_R:
                autoRotate = !autoRotate;
                cout << "Auto rotation: " << (autoRotate ? "ON" : "OFF") << endl;
                break;
            case GLFW_KEY_W:
                wireframeMode = !wireframeMode;
                glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);
                cout << "Wireframe mode: " << (wireframeMode ? "ON" : "OFF") << endl;
                break;
            case GLFW_KEY_SPACE:
                shapeMode = (shapeMode + 1) % 4;
                cout << "Shape mode: " << shapeMode << " (0=Triangle 1=Square 2=Circle 3=Star)" << endl;
                break;
            case GLFW_KEY_UP:
                triangleScale += 0.1f;
                cout << "Scale: " << triangleScale << endl;
                break;
            case GLFW_KEY_DOWN:
                triangleScale = max(0.1f, triangleScale - 0.1f);
                cout << "Scale: " << triangleScale << endl;
                break;
            case GLFW_KEY_LEFT:
                trianglePosition.x -= 0.1f;
                cout << "Position: (" << trianglePosition.x << ", " << trianglePosition.y << ")" << endl;
                break;
            case GLFW_KEY_RIGHT:
                trianglePosition.x += 0.1f;
                cout << "Position: (" << trianglePosition.x << ", " << trianglePosition.y << ")" << endl;
                break;
            case GLFW_KEY_Q:
                trianglePosition.y += 0.1f;
                cout << "Position: (" << trianglePosition.x << ", " << trianglePosition.y << ")" << endl;
                break;
            case GLFW_KEY_E:
                trianglePosition.y -= 0.1f;
                cout << "Position: (" << trianglePosition.x << ", " << trianglePosition.y << ")" << endl;
                break;
            case GLFW_KEY_1:
                triangleColor = glm::vec3(1.0f, 0.0f, 0.0f); // 红色
                cout << "Color changed to: Red" << endl;
                break;
            case GLFW_KEY_2:
                triangleColor = glm::vec3(0.0f, 1.0f, 0.0f); // 绿色
                cout << "Color changed to: Green" << endl;
                break;
            case GLFW_KEY_3:
                triangleColor = glm::vec3(0.0f, 0.0f, 1.0f); // 蓝色
                cout << "Color changed to: Blue" << endl;
                break;
            case GLFW_KEY_4:
                triangleColor = glm::vec3(1.0f, 1.0f, 0.0f); // 黄色
                cout << "Color changed to: Yellow" << endl;
                break;
            case GLFW_KEY_EQUAL:
                animationSpeed += 0.2f;
                cout << "Animation speed: " << animationSpeed << endl;
                break;
            case GLFW_KEY_MINUS:
                animationSpeed = max(0.1f, animationSpeed - 0.2f);
                cout << "Animation speed: " << animationSpeed << endl;
                break;
        }
    }
}

// 鼠标回调函数
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        // 将屏幕坐标转换为NDC坐标
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        trianglePosition.x = (xpos / width) * 2.0f - 1.0f;
        trianglePosition.y = -((ypos / height) * 2.0f - 1.0f);
        
        cout << "Shape moved to: (" << trianglePosition.x << ", " << trianglePosition.y << ")" << endl;
    }
}

// 生成不同形状的顶点数据
vector<float> generateVertices(int mode) {
    vector<float> vertices;
    
    switch(mode) {
        case 0: { // 三角形
            vertices = {
                // 位置坐标        // 颜色值
                -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
                 0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
                 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
            };
            break;
        }
        case 1: { // 正方形
            vertices = {
                // 位置坐标        // 颜色值
                -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
                 0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
                 0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,
                 0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,
                -0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 0.0f,
                -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f
            };
            break;
        }
        case 2: { // 圆形
            const int segments = 36;
            const float radius = 0.5f;
            
            // 中心点
            vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f});
            
            for(int i = 0; i <= segments; i++) {
                float angle = 2.0f * M_PI * i / segments;
                float x = radius * cos(angle);
                float y = radius * sin(angle);
                float r = sin(angle * 3) * 0.5f + 0.5f;
                float g = cos(angle * 2) * 0.5f + 0.5f;
                float b = sin(angle + M_PI) * 0.5f + 0.5f;
                vertices.insert(vertices.end(), {x, y, 0.0f, r, g, b});
            }
            break;
        }
        case 3: { // 五角星
            const float outerRadius = 0.5f;
            const float innerRadius = 0.2f;
            
            vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f});
            
            for(int i = 0; i < 10; i++) {
                float angle = M_PI * i / 5.0f;
                float radius = (i % 2 == 0) ? outerRadius : innerRadius;
                float x = radius * sin(angle);
                float y = radius * cos(angle);
                float intensity = (i % 2 == 0) ? 1.0f : 0.5f;
                vertices.insert(vertices.end(), {x, y, 0.0f, intensity, intensity * 0.5f, 0.0f});
            }
            // 闭合星形
            float x = outerRadius * sin(0);
            float y = outerRadius * cos(0);
            vertices.insert(vertices.end(), {x, y, 0.0f, 1.0f, 0.5f, 0.0f});
            break;
        }
    }
    
    return vertices;
}

// 打印控制说明
void printControls() {
    cout << "\n=== Enhanced OpenGL Graphics Controls ===" << endl;
    cout << "ESC     : Exit program" << endl;
    cout << "R       : Toggle auto rotation" << endl;
    cout << "W       : Toggle wireframe mode" << endl;
    cout << "SPACE   : Switch shapes (Triangle/Square/Circle/Star)" << endl;
    cout << "UP/DOWN : Scale up/down" << endl;
    cout << "LEFT/RIGHT : Move left/right" << endl;
    cout << "Q/E     : Move up/down" << endl;
    cout << "1/2/3/4 : Change color (Red/Green/Blue/Yellow)" << endl;
    cout << "+/-     : Increase/decrease animation speed" << endl;
    cout << "Mouse   : Click to move shape" << endl;
    cout << "==========================================\n" << endl;
}

int main()
{
    // 初始化GLFW
    if(!glfwInit()) {
        cout << "Failed to initialize GLFW" << endl;
        return -1;
    }
    
    // 设置OpenGL版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 抗锯齿
    
    // 创建窗口
    GLFWwindow *window = glfwCreateWindow(1000, 800, "Enhanced OpenGL Graphics - Interactive Shapes", NULL, NULL);
    if(window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    
    // 设置回调函数
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    
    // 设置窗口上下文为当前线程的主上下文
    glfwMakeContextCurrent(window);
    
    // 在确定上下文以后，才能初始化GLEW
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK) {
        cout << "Failed to initialize GLEW" << endl;
        return -1;
    }
    
    // 设置观察的视口
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    // 启用功能
    glEnable(GL_MULTISAMPLE); // 抗锯齿
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 创建并编译顶点着色器
    unsigned int vertexshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexshader, 1, &vertexshadersource, NULL);
    glCompileShader(vertexshader);
    
    // 检查顶点着色器编译是否成功
    int success;
    char infolog[512];
    glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexshader, 512, NULL, infolog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED" << endl;
        cout << infolog << endl;
    }
    
    // 创建并编译片段着色器
    unsigned int fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentshader, 1, &fragmentshadersource, NULL);
    glCompileShader(fragmentshader);
    
    // 检查片段着色器编译是否成功
    glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentshader, 512, NULL, infolog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED" << endl;
        cout << infolog << endl;
    }
    
    // 创建着色器程序对象并且链接
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexshader);
    glAttachShader(shaderProgram, fragmentshader);
    glLinkProgram(shaderProgram);
    
    // 检查链接错误
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infolog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED" << endl;
        cout << infolog << endl;
    }
    
    // 删除着色器对象
    glDeleteShader(vertexshader);
    glDeleteShader(fragmentshader);
    
    // 获取uniform变量位置
    int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    int timeLoc = glGetUniformLocation(shaderProgram, "time");
    int baseColorLoc = glGetUniformLocation(shaderProgram, "baseColor");
    int effectModeLoc = glGetUniformLocation(shaderProgram, "effectMode");
    int waveEffectLoc = glGetUniformLocation(shaderProgram, "waveEffect");
    
    // 创建投影矩阵
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
    
    // 创建VAO和VBO
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    // 打印控制说明
    printControls();
    
    cout << "Enhanced OpenGL Graphics Started Successfully!" << endl;
    cout << "Window Size: " << width << "x" << height << endl;
    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
    cout << "Graphics Renderer: " << glGetString(GL_RENDERER) << endl;
    
    // 渲染循环
    while(!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        
        // 处理输入事件
        glfwPollEvents();
        
        // 自动旋转逻辑
        if(autoRotate) {
            triangleRotation += animationSpeed * 50.0f / 60.0f; // 每秒50度
        }
        
        // 设置动态背景（深色渐变）
        float r = sin(currentTime * 0.3f) * 0.1f + 0.1f;
        float g = cos(currentTime * 0.2f) * 0.1f + 0.15f;
        float b = sin(currentTime * 0.1f) * 0.1f + 0.2f;
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // 生成当前形状的顶点数据
        static int lastShapeMode = -1;
        static vector<float> vertices;
        if(shapeMode != lastShapeMode) {
            vertices = generateVertices(shapeMode);
            lastShapeMode = shapeMode;
            
            // 更新VBO数据
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
            
            // 配置顶点属性指针
            // 位置属性 (location = 0)
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            // 颜色属性 (location = 1)
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
        }
        
        // 使用着色器程序
        glUseProgram(shaderProgram);
        
        // 创建变换矩阵
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(trianglePosition.x, trianglePosition.y, -3.0f));
        transform = glm::rotate(transform, glm::radians(triangleRotation), glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::scale(transform, glm::vec3(triangleScale));
        
        // 传递uniform变量到着色器
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(timeLoc, currentTime * animationSpeed);
        glUniform3fv(baseColorLoc, 1, glm::value_ptr(triangleColor));
        glUniform1i(effectModeLoc, static_cast<int>(currentTime) % 4); // 循环切换效果
        glUniform1f(waveEffectLoc, sin(currentTime * 0.5f) * 0.5f + 0.5f);
        
        // 绑定VAO并绘制图形
        glBindVertexArray(vao);
        
        if(shapeMode == 2 || shapeMode == 3) { // 圆形或星形使用三角扇形
            glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 6);
        } else { // 三角形和正方形使用三角形
            glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);
        }
        
        // 交换前后缓冲区
        glfwSwapBuffers(window);
    }
    
    cout << "Cleaning up OpenGL resources..." << endl;
    
    // 清理OpenGL资源
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);
    
    // 终止GLFW
    glfwTerminate();
    
    cout << "Enhanced OpenGL program terminated successfully!" << endl;
    
    return 0;
}