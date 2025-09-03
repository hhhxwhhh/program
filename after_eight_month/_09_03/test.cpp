// 引入必要的头文件
#include <GL/glew.h>              // GLEW库，用于管理OpenGL函数指针
#include <GLFW/glfw3.h>           // GLFW库，用于创建窗口和处理输入
#include <glm/glm.hpp>            // GLM数学库，用于处理向量和矩阵运算
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"            // STB图像加载库

#include <iostream>
#include <cmath>
using namespace std;

// 函数声明
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// 屏幕宽高常量
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 顶点着色器源码（GLSL语言）
// 顶点着色器负责处理每个顶点的位置、法线和纹理坐标
const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"           // 顶点位置属性，location=0
"layout (location = 1) in vec3 aNormal;\n"        // 顶点法线属性，location=1
"layout (location = 2) in vec2 aTexCoord;\n"      // 纹理坐标属性，location=2
"\n"
"// 输出变量，传递给片段着色器\n"
"out vec3 FragPos;\n"                             // 片段位置（世界坐标）
"out vec3 Normal;\n"                              // 法线向量
"out vec2 TexCoord;\n"                            // 纹理坐标
"\n"
"// 统一变量，从CPU传递给GPU的变换矩阵\n"
"uniform mat4 model;\n"                           // 模型矩阵
"uniform mat4 view;\n"                            // 视图矩阵
"uniform mat4 projection;\n"                      // 投影矩阵
"\n"
"void main()\n"
"{\n"
"    // 计算顶点的最终位置（投影 * 视图 * 模型 * 顶点位置）\n"
"    gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
"    // 计算世界空间中的片段位置\n"
"    FragPos = vec3(model * vec4(aPos, 1.0f));\n"
"    // 法线矩阵变换（用于处理非均匀缩放情况下的法线）\n"
"    Normal = mat3(transpose(inverse(model))) * aNormal;\n"
"    // 直接传递纹理坐标\n"
"    TexCoord = aTexCoord;\n"
"}\0";

// 片段着色器源码（GLSL语言）
// 片段着色器负责计算每个像素的最终颜色
const char *fragmentShaderSource = "#version 330 core\n"
"// 从顶点着色器接收的输入变量\n"
"in vec3 FragPos;\n"                              // 片段位置
"in vec3 Normal;\n"                               // 法线向量
"in vec2 TexCoord;\n"                             // 纹理坐标
"\n"
"out vec4 FragColor;\n"                           // 输出颜色
"\n"
"// 统一变量，从CPU传递给GPU的光照和材质参数\n"
"uniform vec3 objectColor;\n"                     // 物体颜色
"uniform vec3 lightColor;\n"                      // 光源颜色
"uniform vec3 lightPos;\n"                        // 光源位置
"uniform vec3 viewPos;\n"                         // 视点位置
"\n"
"uniform sampler2D ourTexture;\n"                 // 纹理采样器
"uniform bool useTexture;\n"                      // 是否使用纹理的标志

"\n"
"void main()\n"
"{\n"
"    // 环境光计算\n"
"    float ambientStrength = 0.1;\n"              // 环境光强度
"    vec3 ambient = ambientStrength * lightColor;\n"  // 环境光颜色
"    \n"
"    // 漫反射光计算\n"
"    vec3 norm = normalize(Normal);\n"            // 标准化法线向量
"    vec3 lightDir = normalize(lightPos - FragPos);\n"  // 光线方向向量
"    float diff = max(dot(norm, lightDir), 0.0);\n"     // 漫反射因子
"    vec3 diffuse = diff * lightColor;\n"         // 漫反射颜色
"    \n"
"    // 镜面反射光计算\n"
"    float specularStrength = 0.5;\n"             // 镜面反射强度
"    vec3 viewDir = normalize(viewPos - FragPos);\n"    // 视线方向向量
"    vec3 reflectDir = reflect(-lightDir, norm);\n"     // 反射方向向量
"    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);\n"  // 镜面反射因子
"    vec3 specular = specularStrength * spec * lightColor;\n"     // 镜面反射颜色
"    \n"
"    // 最终颜色计算\n"
"    vec3 result;\n"
"    if (useTexture) {\n"
"        // 如果使用纹理，则从纹理中采样颜色\n"
"        vec4 textureColor = texture(ourTexture, TexCoord);\n"
"        result = (ambient + diffuse + specular) * vec3(textureColor);\n"
"    } else {\n"
"        // 如果不使用纹理，则使用物体本身颜色\n"
"        result = (ambient + diffuse + specular) * objectColor;\n"
"    }\n"
"    FragColor = vec4(result, 1.0);\n"            // 输出最终颜色
"}\n\0";

int main()
{
    // 初始化GLFW库
    glfwInit();
    // 设置OpenGL版本为3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // 使用核心模式（不包含遗留功能）
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口对象
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "光照与纹理示例(GLEW版本)", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();  // 终止GLFW
        return -1;
    }
    // 将窗口的上下文设置为当前线程的主上下文
    glfwMakeContextCurrent(window);
    // 注册窗口大小改变的回调函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 初始化GLEW，必须在创建OpenGL上下文之后调用
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // 启用深度测试，用于正确处理3D物体的前后关系
    glEnable(GL_DEPTH_TEST);

    // 编译顶点着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);  // 创建顶点着色器对象
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);    // 设置着色器源码
    glCompileShader(vertexShader);                                 // 编译着色器
    
    // 检查顶点着色器编译错误
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // 编译片段着色器
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);  // 创建片段着色器对象
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);    // 设置着色器源码
    glCompileShader(fragmentShader);                                   // 编译着色器
    
    // 检查片段着色器编译错误
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // 链接着色器程序
    unsigned int shaderProgram = glCreateProgram();         // 创建着色器程序对象
    glAttachShader(shaderProgram, vertexShader);            // 将顶点着色器附加到程序对象
    glAttachShader(shaderProgram, fragmentShader);          // 将片段着色器附加到程序对象
    glLinkProgram(shaderProgram);                           // 链接着色器程序
    
    // 检查着色器程序链接错误
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    // 删除着色器对象（它们已经链接到程序中，不再需要）
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 定义立方体的顶点数据（位置、法线、纹理坐标）
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    // 创建顶点数组对象(VAO)和顶点缓冲对象(VBO)
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);  // 生成1个VAO
    glGenBuffers(1, &VBO);       // 生成1个VBO

    // 绑定VAO（记录后续的顶点属性配置）
    glBindVertexArray(VAO);

    // 绑定VBO并传输顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 设置顶点属性指针
    // 位置属性 (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 法线属性 (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // 纹理坐标属性 (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // 加载并创建纹理
    unsigned int texture1, texture2;
    glGenTextures(1, &texture1);           // 生成纹理对象
    glBindTexture(GL_TEXTURE_2D, texture1);  // 绑定纹理对象
    // 设置纹理环绕和过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // 加载图像，创建纹理
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);  // 翻转图像的Y轴
    unsigned char *data = stbi_load("D:/OpenGL/program/after_eight_month/_09_03/2.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        // 根据图像的通道数确定纹理格式
        GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
            
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);  // 生成多级渐远纹理
    }
    else
    {
        std::cout << "Failed to load texture1" << std::endl;
    }
    stbi_image_free(data);  // 释放图像内存

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // 为当前绑定的纹理对象设置环绕、过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 加载第二张图像
    data = stbi_load("D:/OpenGL/program/after_eight_month/_09_03/1.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
            
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture2" << std::endl;
    }
    stbi_image_free(data);

    // 告诉opengl每个着色器采样器属于哪个纹理单元
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture"), 0);

    // 渲染循环（主循环）
    while (!glfwWindowShouldClose(window))
    {
        // 处理输入事件
        processInput(window);

        // 清除屏幕颜色缓冲和深度缓冲
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 激活着色器程序
        glUseProgram(shaderProgram);

        // 绑定纹理到纹理单元0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        // 设置模型、视图和投影矩阵
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        
        // 传递变换矩阵到着色器
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // 设置摄像机位置
        unsigned int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
        glUniform3f(viewPosLoc, 0.0f, 0.0f, 3.0f);

        // 创建移动光源（光源在场景中做圆周运动）
        float time = glfwGetTime();
        float lightX = sin(time * 0.5f) * 3.0f;
        float lightZ = cos(time * 0.5f) * 3.0f;
        glm::vec3 lightPos(lightX, 1.0f, lightZ);
        
        // 传递光源位置和颜色到着色器
        unsigned int lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        unsigned int lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
        glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);

        // 绘制多个立方体
        glm::vec3 cubePositions[] = {
            glm::vec3( 0.0f,  0.0f,  0.0f),
            glm::vec3( 2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3( 2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3( 1.3f, -2.0f, -2.5f),
            glm::vec3( 1.5f,  2.0f, -2.5f),
            glm::vec3( 1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
        };

        for (unsigned int i = 0; i < 10; i++)
        {
            // 计算每个立方体的模型矩阵
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            
            // 旋转立方体（随时间旋转）
            float angle = 20.0f * i + time * 20.0f;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            
            // 传递模型矩阵到着色器
            unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            // 根据索引决定使用纹理还是纯色
            if (i % 3 == 0) {
                // 使用第一种纹理
                glBindTexture(GL_TEXTURE_2D, texture1);
                glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1);
            } else if (i % 3 == 1) {
                // 使用第二种纹理
                glBindTexture(GL_TEXTURE_2D, texture2);
                glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1);
            } else {
                // 使用纯色
                glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
                // 设置物体颜色（每个立方体不同颜色）
                glm::vec3 colors[] = {
                    glm::vec3(1.0f, 0.5f, 0.31f), // 红橙色
                    glm::vec3(1.0f, 1.0f, 1.0f),   // 白色
                    glm::vec3(1.0f, 0.5f, 0.5f),   // 粉色
                    glm::vec3(0.5f, 1.0f, 0.5f),   // 绿色
                    glm::vec3(0.5f, 0.5f, 1.0f),   // 蓝色
                    glm::vec3(1.0f, 1.0f, 0.0f),   // 黄色
                    glm::vec3(1.0f, 0.0f, 1.0f),   // 紫色
                    glm::vec3(0.0f, 1.0f, 1.0f),   // 青色
                    glm::vec3(0.5f, 0.5f, 0.5f),   // 灰色
                    glm::vec3(0.2f, 0.8f, 0.6f)    // 青绿色
                };
                
                unsigned int objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
                glUniform3f(objectColorLoc, colors[i].x, colors[i].y, colors[i].z);
            }

            // 绘制立方体（36个顶点构成两个三角形组成的6个面）
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 交换前后缓冲区（双缓冲）
        glfwSwapBuffers(window);
        // 处理事件队列
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // 终止GLFW，释放所有分配的GLFW资源
    glfwTerminate();
    return 0;
}

// 处理所有输入：查询GLFW是否按下相关按键并做出相应反应
void processInput(GLFWwindow *window)
{
    // 如果按下ESC键，则设置窗口应该关闭
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// 当窗口大小改变时，调用这个回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // 设置视口大小，匹配新的窗口尺寸
    glViewport(0, 0, width, height);
}

// 加载纹理的辅助函数
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);  // 生成纹理ID

    int width, height, nrComponents;
    // 使用stb_image库加载图像
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        // 根据图像的通道数确定格式
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        // 绑定纹理并设置图像数据
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);  // 生成多级渐远纹理

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);  // 释放图像内存
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}