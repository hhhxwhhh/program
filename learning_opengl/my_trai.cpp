#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <cstdlib>
#include <cmath>
using namespace std;

// 🎨 修改着色器源码 - 添加时间uniform
const char* vertexShaderSource = R"(
#version 430 core
layout (location = 0) in vec3 aPos;

out vec3 vertexColor;

uniform float time;  // 时间uniform

void main()
{
    gl_Position = vec4(aPos, 1.0);
    
    // 基于时间和顶点位置计算动态颜色
    float r = 0.5 + 0.5 * sin(time * 2.0 + aPos.x * 3.14159);
    float g = 0.5 + 0.5 * sin(time * 2.0 + aPos.y * 3.14159 + 2.094);  // +2π/3
    float b = 0.5 + 0.5 * sin(time * 2.0 + (aPos.x + aPos.y) * 3.14159 + 4.188);  // +4π/3
    
    vertexColor = vec3(r, g, b);
}
)";

const char* fragmentShaderSource = R"(
#version 430 core
in vec3 vertexColor;
out vec4 FragColor;

uniform float time;  // 也可以在片段着色器中使用时间

void main()
{
    // 可选：添加闪烁效果
    float pulse = 0.8 + 0.2 * sin(time * 8.0);
    FragColor = vec4(vertexColor * pulse, 1.0);
}
)";

//读取文件内容
string read_file(const string &file_path)
{
    ifstream file(file_path);
    if(!file.is_open())
    {
        cerr << "Failed to open file: " << file_path << endl;
        return "";
    }
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    return content;
}

// 从字符串创建着色器程序
static unsigned int create_program_from_source(const char* vertex_source, const char* fragment_source)
{
    //创建shader
    unsigned int vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader_id, 1, &vertex_source, NULL);
    
    unsigned int fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader_id, 1, &fragment_source, NULL);
    
    //编译shader
    glCompileShader(vertex_shader_id);
    int success;
    char infolog[512];
    glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex_shader_id, 512, NULL, infolog);
        cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infolog << endl;
    }
    
    glCompileShader(fragment_shader_id);
    glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragment_shader_id, 512, NULL, infolog);
        cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infolog << endl;
    }
    
    //链接着色器程序
    unsigned int shader_program_id = glCreateProgram();
    glAttachShader(shader_program_id, vertex_shader_id);
    glAttachShader(shader_program_id, fragment_shader_id);
    glLinkProgram(shader_program_id);
    
    // 检查链接状态
    glGetProgramiv(shader_program_id, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shader_program_id, 512, NULL, infolog);
        cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infolog << endl;
    }   
    
    //删除着色器对象
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);
    return shader_program_id;
}

//创建着色器程序（从文件）
static unsigned int create_program(const string &vertex_shader_path, const string &fragment_shader_path)
{
    //读取shader文件
    string vertex_shader_source = read_file(vertex_shader_path);
    string fragment_shader_source = read_file(fragment_shader_path);
    
    // 如果文件读取失败，使用内嵌着色器
    if(vertex_shader_source.empty() || fragment_shader_source.empty()) {
        cout << "Using embedded shaders..." << endl;
        return create_program_from_source(vertexShaderSource, fragmentShaderSource);
    }
    
    return create_program_from_source(vertex_shader_source.c_str(), fragment_shader_source.c_str());
}

int main()
{
    //初始化GLFW
    if(!glfwInit())
    {
        cerr << "Failed to initialize GLFW" << endl;
        return -1;
    }
    
    //设置OpenGL版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    //创建窗口
    GLFWwindow *window = glfwCreateWindow(800, 600, "OpenGL Dynamic Colors", NULL, NULL);
    if(!window)
    {
        cerr << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    
    //设置当前上下文
    glfwMakeContextCurrent(window);
    
    //初始化GLEW
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
    {
        cerr << "Failed to initialize GLEW" << endl;
        return -1;
    }
    
    //设置视口
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    // 打印OpenGL信息
    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
    
    //创建着色器程序
    unsigned int shader_program_id = create_program("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    
    // 顶点数据
    float vertices[] = {
        // 位置坐标
        -0.5f, -0.5f, 0.0f,  // 左下
         0.5f, -0.5f, 0.0f,  // 右下
         0.5f,  0.5f, 0.0f,  // 右上
        -0.5f,  0.5f, 0.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3};
    //创建VAO和VBO
    unsigned int VAO, VBO,EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // 绑定并填充VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // 绑定并填充EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 设置位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 🎯 获取uniform位置
    int timeLocationVertex = glGetUniformLocation(shader_program_id, "time");
    if(timeLocationVertex == -1) {
        cout << "Warning: Could not find 'time' uniform in vertex shader" << endl;
    }
    
    // 检查片段着色器中的time uniform
    glUseProgram(shader_program_id);  // 需要先使用程序
    int timeLocationFragment = glGetUniformLocation(shader_program_id, "time");
    
    cout << "Vertex shader time location: " << timeLocationVertex << endl;
    cout << "Fragment shader time location: " << timeLocationFragment << endl;
    cout << "Starting render loop with dynamic colors..." << endl;
    
    //渲染循环
    while(!glfwWindowShouldClose(window))
    {
        //处理输入
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }
        
        // 🎨 获取当前时间
        float currentTime = static_cast<float>(glfwGetTime());
        
        // 设置背景色 - 也可以基于时间变化
        float bgR = 0.1f + 0.1f * sin(currentTime * 0.5f);
        float bgG = 0.1f + 0.1f * sin(currentTime * 0.5f + 2.094f);
        float bgB = 0.1f + 0.1f * sin(currentTime * 0.5f + 4.188f);
        glClearColor(bgR, bgG, bgB, 1.0f);
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        //使用着色器程序
        glUseProgram(shader_program_id);
        
        // 🎯 传递时间uniform到着色器
        if(timeLocationVertex != -1) {
            glUniform1f(timeLocationVertex, currentTime);
        }
        
        //绘制三角形
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        //交换缓冲区
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // 清理资源
    glDeleteProgram(shader_program_id);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &EBO);

    //终止GLFW
    glfwTerminate();
    
    cout << "Program terminated successfully!" << endl;
    return 0;
}