#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <fstream>
using namespace glm;
using namespace std;

// 创建着色器程序函数
static unsigned int create_program(const string &vertex_shader,const string &fragment_shader)
{
    // 创建顶点着色器
    unsigned int vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    const char *vertex_shader_source = vertex_shader.c_str();
    glShaderSource(vertex_shader_id, 1, &vertex_shader_source, NULL);
    
    // 创建片段着色器
    unsigned int fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragment_shader_source = fragment_shader.c_str();
    glShaderSource(fragment_shader_id, 1, &fragment_shader_source, NULL);
    
    // 编译顶点着色器
    glCompileShader(vertex_shader_id);
    int success;
    char infoLog[512];
    glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &success);
    if (!success) 
    {
        glGetShaderInfoLog(vertex_shader_id, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // 编译片段着色器
    glCompileShader(fragment_shader_id);
    glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &success);
    if (!success) 
    {
        glGetShaderInfoLog(fragment_shader_id, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // 链接着色器程序
    unsigned int program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);
    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (!success) 
    {
        glGetProgramInfoLog(program_id, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    // 清理着色器对象
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);
    
    return program_id;
}

// 从文件读取着色器源码
string read_file(const string &file_path)
{
    ifstream file(file_path);
    if(!file.is_open())
    {
        cerr << "ERROR::FILE::" << file_path << " NOT FOUND" << endl;
        return "";
    }
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    return content;
}

// 键盘输入回调函数
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main()
{
    // 初始化GLFW
    if(!glfwInit())
    {
        cerr << "ERROR::GLFW::INITIALIZATION_FAILED" << endl;
        return -1;
    }
    
    // 设置GLFW窗口提示
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // 创建GLFW窗口
    GLFWwindow *window = glfwCreateWindow(800, 600, "OpenGL File Shader Loading", NULL, NULL);
    if(!window)
    {
        cerr << "ERROR::GLFW::WINDOW_CREATION_FAILED" << endl;
        glfwTerminate();
        return -1;
    }
    
    // 设置当前窗口上下文
    glfwMakeContextCurrent(window);
    
    // 设置键盘回调
    glfwSetKeyCallback(window, key_callback);
    
    // 初始化GLEW
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
    {
        cerr << "ERROR::GLEW::INITIALIZATION_FAILED" << endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    // 设置视口
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
    
    // 从文件读取着色器源码
    string vertex_shader_source = read_file("shaders/vertex.glsl");
    string fragment_shader_source = read_file("shaders/fragment.glsl");
    
    // 如果文件读取失败，使用默认着色器
    if(vertex_shader_source.empty()) {
        cout << "Using default vertex shader..." << endl;
        vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";
    }
    
    if(fragment_shader_source.empty()) {
        cout << "Using default fragment shader..." << endl;
        fragment_shader_source = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

uniform float time;

void main()
{
    // 基础颜色加上时间变化效果
    vec3 color = vertexColor;
    color += 0.2 * sin(time * 2.0);
    FragColor = vec4(color, 1.0);
}
)";
    }
    
    // 创建着色器程序
    unsigned int shader_program = create_program(vertex_shader_source, fragment_shader_source);
    
    // 定义三角形顶点数据（位置 + 颜色）
    float vertices[] = {
        // 位置           // 颜色
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // 左下 - 红色
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // 右下 - 绿色
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // 顶部 - 蓝色
    };
    
    // 创建VAO和VBO
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    // 绑定VAO
    glBindVertexArray(VAO);
    
    // 绑定并填充VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // 配置顶点属性指针 - 位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 配置顶点属性指针 - 颜色
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 解绑
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // 获取uniform位置
    int transform_location = glGetUniformLocation(shader_program, "transform");
    int time_location = glGetUniformLocation(shader_program, "time");
    
    cout << "Shader program created successfully!" << endl;
    cout << "Press ESC to exit" << endl;
    
    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 处理输入事件
        glfwPollEvents();
        
        // 获取当前时间
        float current_time = glfwGetTime();
        
        // 清除屏幕
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // 使用着色器程序
        glUseProgram(shader_program);
        
        // 创建变换矩阵（旋转动画）
        mat4 transform = mat4(1.0f);
        transform = rotate(transform, current_time, vec3(0.0f, 0.0f, 1.0f));
        transform = scale(transform, vec3(0.8f, 0.8f, 0.8f));
        
        // 传递uniform变量
        glUniformMatrix4fv(transform_location, 1, GL_FALSE, value_ptr(transform));
        glUniform1f(time_location, current_time);
        
        // 绘制三角形
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        // 交换缓冲区
        glfwSwapBuffers(window);
    }
    
    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shader_program);
    
    // 销毁窗口并终止GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    
    cout << "Program terminated successfully!" << endl;
    return 0;
}