// ===============================================================================
// 📋 OpenGL 动态彩色正方形渲染程序
// 功能: 使用着色器实现基于时间的动态颜色变化效果
// 作者: [wxf]
// 创建时间: [2025年5月]
// ===============================================================================

// 🔧 引入必要的头文件
#include <iostream>      // 标准输入输出流
#include <GL/glew.h>     // OpenGL扩展加载库
#include <GLFW/glfw3.h>  // 窗口管理和输入处理
#include <fstream>       // 文件流操作
#include <cstdlib>       // 标准库函数
#include <cmath>         // 数学函数（sin, cos等）
using namespace std;

// ===============================================================================
// 🎨 着色器源码定义
// ===============================================================================

// 🖥️ 顶点着色器 - 处理每个顶点的位置和颜色计算
const char* vertexShaderSource = R"(
#version 430 core
layout (location = 0) in vec3 aPos;        // 输入：顶点位置属性

out vec3 vertexColor;                       // 输出：传递给片段着色器的颜色

uniform float time;                         // Uniform：时间变量

void main()
{
    // 📍 设置顶点的最终位置（模型 -> 世界 -> 裁剪坐标）
    gl_Position = vec4(aPos, 1.0);
    
    // 🌈 基于时间和顶点位置计算动态颜色
    // 使用正弦函数创建平滑的颜色过渡效果
    float r = 0.5 + 0.5 * sin(time * 2.0 + aPos.x * 3.14159);                    // 红色分量
    float g = 0.5 + 0.5 * sin(time * 2.0 + aPos.y * 3.14159 + 2.094);            // 绿色分量 (+2π/3相位偏移)
    float b = 0.5 + 0.5 * sin(time * 2.0 + (aPos.x + aPos.y) * 3.14159 + 4.188); // 蓝色分量 (+4π/3相位偏移)
    
    vertexColor = vec3(r, g, b);
}
)";

// 🎨 片段着色器 - 处理每个像素的最终颜色
const char* fragmentShaderSource = R"(
#version 430 core
in vec3 vertexColor;                        // 输入：从顶点着色器插值得到的颜色

out vec4 FragColor;                         // 输出：最终像素颜色

uniform float time;                         // Uniform：时间变量

void main()
{
    // ✨ 添加脉冲效果 - 让整体亮度周期性变化
    float pulse = 0.8 + 0.2 * sin(time * 8.0);  // 脉冲系数：0.6 ~ 1.0
    
    // 🎯 输出最终颜色（RGB + Alpha通道）
    FragColor = vec4(vertexColor * pulse, 1.0);
}
)";

// ===============================================================================
// 🛠️ 工具函数定义
// ===============================================================================

// 📂 读取文件内容到字符串
string read_file(const string &file_path)
{
    ifstream file(file_path);
    if(!file.is_open())
    {
        cerr << "❌ Failed to open file: " << file_path << endl;
        return "";
    }
    
    // 📖 使用迭代器读取整个文件内容
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    return content;
}

// 🔨 从源码字符串创建着色器程序
static unsigned int create_program_from_source(const char* vertex_source, const char* fragment_source)
{
    // 📝 第一步：创建着色器对象
    unsigned int vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader_id, 1, &vertex_source, NULL);
    
    unsigned int fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader_id, 1, &fragment_source, NULL);
    
    // 🔧 第二步：编译着色器
    int success;
    char infolog[512];
    
    // 编译顶点着色器
    glCompileShader(vertex_shader_id);
    glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex_shader_id, 512, NULL, infolog);
        cerr << "❌ ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infolog << endl;
    }
    
    // 编译片段着色器
    glCompileShader(fragment_shader_id);
    glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragment_shader_id, 512, NULL, infolog);
        cerr << "❌ ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infolog << endl;
    }
    
    // 🔗 第三步：创建着色器程序并链接
    unsigned int shader_program_id = glCreateProgram();
    glAttachShader(shader_program_id, vertex_shader_id);     // 附加顶点着色器
    glAttachShader(shader_program_id, fragment_shader_id);   // 附加片段着色器
    glLinkProgram(shader_program_id);                        // 链接程序
    
    // 检查链接状态
    glGetProgramiv(shader_program_id, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shader_program_id, 512, NULL, infolog);
        cerr << "❌ ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infolog << endl;
    }
    
    // 🗑️ 第四步：清理着色器对象（已经链接到程序中，不再需要）
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);
    
    return shader_program_id;
}

// 📁 从文件创建着色器程序（支持回退到内嵌着色器）
static unsigned int create_program(const string &vertex_shader_path, const string &fragment_shader_path)
{
    // 📖 尝试读取外部着色器文件
    string vertex_shader_source = read_file(vertex_shader_path);
    string fragment_shader_source = read_file(fragment_shader_path);
    
    // 🔄 如果文件读取失败，使用内嵌着色器作为备用方案
    if(vertex_shader_source.empty() || fragment_shader_source.empty()) {
        cout << "⚠️  External shader files not found, using embedded shaders..." << endl;
        return create_program_from_source(vertexShaderSource, fragmentShaderSource);
    }
    
    cout << "✅ Using external shader files..." << endl;
    return create_program_from_source(vertex_shader_source.c_str(), fragment_shader_source.c_str());
}

// ===============================================================================
// 🚀 主程序入口
// ===============================================================================
int main()
{
    // ═══════════════════════════════════════════════════════════════════════════
    // 🏗️ 第一阶段：系统初始化
    // ═══════════════════════════════════════════════════════════════════════════
    
    // 🔧 初始化GLFW库
    if(!glfwInit())
    {
        cerr << "❌ Failed to initialize GLFW" << endl;
        return -1;
    }
    cout << "✅ GLFW initialized successfully" << endl;
    
    // ⚙️ 设置OpenGL版本和核心模式
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);                     // OpenGL 4.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                     // OpenGL x.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);     // 核心模式
    
    // 🪟 创建窗口
    GLFWwindow *window = glfwCreateWindow(800, 600, "🌈 OpenGL Dynamic Square", NULL, NULL);
    if(!window)
    {
        cerr << "❌ Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    cout << "✅ Window created successfully (800x600)" << endl;
    
    // 🎯 设置当前OpenGL上下文
    glfwMakeContextCurrent(window);
    
    // 🔌 初始化GLEW扩展加载器
    glewExperimental = GL_TRUE;  // 启用实验性功能
    if(glewInit() != GLEW_OK)
    {
        cerr << "❌ Failed to initialize GLEW" << endl;
        return -1;
    }
    cout << "✅ GLEW initialized successfully" << endl;
    
    // 📐 设置视口（渲染区域）
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);
    cout << "✅ Viewport set to " << width << "x" << height << endl;
    
    // 📊 打印OpenGL版本信息
    cout << "\n🔍 System Information:" << endl;
    cout << "   OpenGL Version: " << glGetString(GL_VERSION) << endl;
    cout << "   GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
    cout << "   GPU Vendor: " << glGetString(GL_VENDOR) << endl;
    cout << "   GPU Renderer: " << glGetString(GL_RENDERER) << endl;
    
    // ═══════════════════════════════════════════════════════════════════════════
    // 🎨 第二阶段：着色器程序创建
    // ═══════════════════════════════════════════════════════════════════════════
    
    cout << "\n🎨 Creating shader program..." << endl;
    unsigned int shader_program_id = create_program("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    
    // ═══════════════════════════════════════════════════════════════════════════
    // 📐 第三阶段：几何数据准备
    // ═══════════════════════════════════════════════════════════════════════════
    
    cout << "\n📐 Setting up geometry data..." << endl;
    
    // 🔲 正方形顶点数据（4个唯一顶点）
    float vertices[] = {
        // 位置坐标 (X, Y, Z)
        -0.5f, -0.5f, 0.0f,  // 顶点0：左下角
         0.5f, -0.5f, 0.0f,  // 顶点1：右下角
         0.5f,  0.5f, 0.0f,  // 顶点2：右上角
        -0.5f,  0.5f, 0.0f   // 顶点3：左上角
    };
    
    // 🔺 索引数据（定义两个三角形组成正方形）
    unsigned int indices[] = {
        0, 1, 2,   // 第一个三角形：左下 -> 右下 -> 右上
        2, 3, 0    // 第二个三角形：右上 -> 左上 -> 左下
    };
    
    // 📦 创建OpenGL缓冲对象
    unsigned int VAO, VBO, EBO;  // VAO=顶点数组对象, VBO=顶点缓冲对象, EBO=索引缓冲对象
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    cout << "✅ Buffer objects created (VAO:" << VAO << ", VBO:" << VBO << ", EBO:" << EBO << ")" << endl;
    
    // 🔗 绑定VAO（记录所有后续的缓冲区和属性配置）
    glBindVertexArray(VAO);
    
    // 📤 上传顶点数据到GPU显存
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    cout << "✅ Vertex data uploaded to GPU (" << sizeof(vertices) << " bytes)" << endl;
    
    // 📤 上传索引数据到GPU显存
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    cout << "✅ Index data uploaded to GPU (" << sizeof(indices) << " bytes)" << endl;
    
    // 🏷️ 配置顶点属性指针（告诉OpenGL如何解释顶点数据）
    glVertexAttribPointer(
        0,                          // 属性位置（对应着色器中的location = 0）
        3,                          // 每个顶点3个分量（X, Y, Z）
        GL_FLOAT,                   // 数据类型为浮点数
        GL_FALSE,                   // 不需要标准化
        3 * sizeof(float),          // 步长：每个顶点占用3个float的空间
        (void*)0                    // 偏移量：从缓冲区开头开始
    );
    glEnableVertexAttribArray(0);   // 启用属性0
    cout << "✅ Vertex attribute configured (position at location 0)" << endl;
    
    // ═══════════════════════════════════════════════════════════════════════════
    // 🎛️ 第四阶段：Uniform变量配置
    // ═══════════════════════════════════════════════════════════════════════════
    
    cout << "\n🎛️ Configuring shader uniforms..." << endl;
    
    // 🔍 获取着色器中uniform变量的位置
    glUseProgram(shader_program_id);  // 必须先激活着色器程序
    
    int timeLocationVertex = glGetUniformLocation(shader_program_id, "time");
    if(timeLocationVertex == -1) {
        cout << "⚠️  Warning: Could not find 'time' uniform in vertex shader" << endl;
    } else {
        cout << "✅ Found 'time' uniform at location: " << timeLocationVertex << endl;
    }
    
    int timeLocationFragment = glGetUniformLocation(shader_program_id, "time");
    cout << "📍 Uniform locations - Vertex: " << timeLocationVertex 
         << ", Fragment: " << timeLocationFragment << endl;
    
    // ═══════════════════════════════════════════════════════════════════════════
    // 🔄 第五阶段：主渲染循环
    // ═══════════════════════════════════════════════════════════════════════════
    
    cout << "\n🔄 Starting main render loop..." << endl;
    cout << "💡 Press ESC to exit" << endl;
    
    // 📊 性能计数器
    int frame_count = 0;
    double last_time = glfwGetTime();
    
    while(!glfwWindowShouldClose(window))
    {
        // ───────────────────────────────────────────────────────────────────────
        // 🎮 输入处理
        // ───────────────────────────────────────────────────────────────────────
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            cout << "🚪 ESC pressed, exiting..." << endl;
            glfwSetWindowShouldClose(window, true);
        }
        
        // ───────────────────────────────────────────────────────────────────────
        // ⏰ 时间更新
        // ───────────────────────────────────────────────────────────────────────
        float currentTime = static_cast<float>(glfwGetTime());
        
        // ───────────────────────────────────────────────────────────────────────
        // 🎨 背景色设置（基于时间的动态变化）
        // ───────────────────────────────────────────────────────────────────────
        float bgR = 0.1f + 0.1f * sin(currentTime * 0.5f);                    // 红色分量
        float bgG = 0.1f + 0.1f * sin(currentTime * 0.5f + 2.094f);           // 绿色分量
        float bgB = 0.1f + 0.1f * sin(currentTime * 0.5f + 4.188f);           // 蓝色分量
        glClearColor(bgR, bgG, bgB, 1.0f);
        
        // 🧹 清除颜色缓冲区
        glClear(GL_COLOR_BUFFER_BIT);
        
        // ───────────────────────────────────────────────────────────────────────
        // 🖼️ 渲染几何体
        // ───────────────────────────────────────────────────────────────────────
        
        // 🎯 激活着色器程序
        glUseProgram(shader_program_id);
        
        // 📤 传递时间uniform到着色器
        if(timeLocationVertex != -1) {
            glUniform1f(timeLocationVertex, currentTime);
        }
        
        // 🔲 绘制正方形
        glBindVertexArray(VAO);                                    // 绑定顶点数组对象
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);       // 绘制6个顶点（2个三角形）
        
        // ───────────────────────────────────────────────────────────────────────
        // 🔄 缓冲区交换和事件处理
        // ───────────────────────────────────────────────────────────────────────
        glfwSwapBuffers(window);    // 交换前后缓冲区（显示渲染结果）
        glfwPollEvents();           // 处理窗口事件
        
        // 📊 性能统计（每秒输出一次FPS）
        frame_count++;
        double current_time = glfwGetTime();
        if(current_time - last_time >= 1.0) {
            cout << "🎯 FPS: " << frame_count << " | Time: " << (int)current_time << "s" << endl;
            frame_count = 0;
            last_time = current_time;
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════════
    // 🧹 第六阶段：资源清理
    // ═══════════════════════════════════════════════════════════════════════════
    
    cout << "\n🧹 Cleaning up resources..." << endl;
    
    // 🗑️ 删除OpenGL对象
    glDeleteProgram(shader_program_id);   // 删除着色器程序
    glDeleteBuffers(1, &VBO);             // 删除顶点缓冲对象
    glDeleteBuffers(1, &EBO);             // 删除索引缓冲对象
    glDeleteVertexArrays(1, &VAO);        // 删除顶点数组对象
    cout << "✅ OpenGL resources cleaned up" << endl;
    
    // 🏁 终止GLFW
    glfwTerminate();
    cout << "✅ GLFW terminated" << endl;
    
    cout << "\n🎉 Program terminated successfully!" << endl;
    return 0;
}

// ===============================================================================
// 📚 程序说明文档
// ===============================================================================
/*
🎯 程序功能：
   - 创建一个800x600的OpenGL窗口
   - 渲染一个动态变色的正方形
   - 实现基于时间的颜色动画效果
   - 支持ESC键退出

🔧 技术要点：
   1. 使用EBO（索引缓冲对象）优化渲染
   2. 在顶点着色器中计算动态颜色
   3. 添加脉冲亮度效果
   4. 动态背景色变化

📁 文件结构：
   my_trai.cpp              - 主程序文件
   shaders/vertex_shader.glsl    - 外部顶点着色器（可选）
   shaders/fragment_shader.glsl  - 外部片段着色器（可选）

🎮 控制方式：
   ESC - 退出程序

🔗 依赖库：
   - GLFW3  (窗口管理)
   - GLEW   (OpenGL扩展)
   - OpenGL 4.3+ (图形渲染)

💡 编译命令：
   g++ -o my_trai my_trai.cpp -lglfw3 -lglew32 -lopengl32 -lgdi32
*/