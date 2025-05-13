#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <direct.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

#define numvaos 1

GLuint vao[numvaos];
GLuint renderingprogram;

// 打印着色器编译错误
void printShaderLog(GLuint shader)
{
    int len = 0;
    int chWritten = 0;
    char *log;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len > 0)
    {
        log = new char[len];
        glGetShaderInfoLog(shader, len, &chWritten, log);
        cout << "Shader Info Log: " << log << endl;
        delete[] log;
    }
}

// 打印程序链接错误
void printProgramLog(GLuint program)
{
    int len = 0;
    int chWritten = 0;
    char *log;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
    if (len > 0)
    {
        log = new char[len];
        glGetProgramInfoLog(program, len, &chWritten, log);
        cout << "Program Info Log: " << log << endl;
        delete[] log;
    }
}

// 读取着色器源代码
string readShaderSource(const char *filename)
{
    ifstream fileStream(filename, ios::in);
    if (!fileStream.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        char cwd[1024];
        if (_getcwd(cwd, sizeof(cwd)) != NULL)
        {
            cerr << "Current working directory: " << cwd << endl;
        }
        return "";
    }

    string content((istreambuf_iterator<char>(fileStream)), istreambuf_iterator<char>());
    fileStream.close();
    cout << "Shader file read: " << filename << endl;
    return content;
}

// create shader program
GLuint createshaderprogram()
{
    // 从文件中读取着色器代码
    string vertShaderStr = readShaderSource("vertShader.glsl");
    string fragShaderStr = readShaderSource("fragShader.glsl");

    // 检查着色器代码是否成功读取
    if (vertShaderStr.empty() || fragShaderStr.empty())
    {
        cerr << "Failed to read shader files. Using hardcoded shaders as fallback." << endl;

        // 使用硬编码的着色器作为备选
        vertShaderStr =
            "#version 410 core\n"
            "void main()\n"
            "{\n"
            "    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
            "}\n";

        fragShaderStr =
            "#version 410 core\n"
            "out vec4 fragColor;\n"
            "void main()\n"
            "{\n"
            "    fragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
            "}\n";
    }

    const char *vShaderSource = vertShaderStr.c_str();
    const char *fShaderSource = fragShaderStr.c_str();

    // 创建着色器对象
    GLuint vertexshader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);

    // attach code with the shader object
    glShaderSource(vertexshader, 1, &vShaderSource, NULL);
    glShaderSource(fragmentshader, 1, &fShaderSource, NULL);

    // compile the shader
    glCompileShader(vertexshader);
    GLint vertCompiled;
    glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &vertCompiled);
    if (vertCompiled != GL_TRUE)
    {
        cerr << "Vertex shader compilation failed." << endl;
        printShaderLog(vertexshader);
        return 0;
    }

    glCompileShader(fragmentshader);
    GLint fragCompiled;
    glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &fragCompiled);
    if (fragCompiled != GL_TRUE)
    {
        cerr << "Fragment shader compilation failed." << endl;
        printShaderLog(fragmentshader);
        return 0;
    }

    // create the program object
    GLuint program = glCreateProgram();

    // attach the shaders to the program
    glAttachShader(program, vertexshader);
    glAttachShader(program, fragmentshader);

    // link
    glLinkProgram(program);
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE)
    {
        cerr << "Shader program linking failed." << endl;
        printProgramLog(program);
        return 0;
    }

    // 清理着色器对象
    glDetachShader(program, vertexshader);
    glDetachShader(program, fragmentshader);
    glDeleteShader(vertexshader);
    glDeleteShader(fragmentshader);

    return program;
}

void init(GLFWwindow *window)
{
    cout << "Initializing program..." << endl;
    renderingprogram = createshaderprogram();
    if (renderingprogram == 0)
    {
        cerr << "Failed to create rendering program." << endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    cout << "Rendering program created successfully: " << renderingprogram << endl;

    glGenVertexArrays(numvaos, vao);
    glBindVertexArray(vao[0]);
    cout << "VAO created and bound." << endl;
}

void display(GLFWwindow *window, double time)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 黑色背景
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(renderingprogram);
    glPointSize(100.0f);
    glDrawArrays(GL_POINTS, 0, 1);
}

int main(void)
{
    cout << "Program started." << endl;

    // 打印当前工作目录
    char cwd[1024];
    if (_getcwd(cwd, sizeof(cwd)) != NULL)
    {
        cout << "Current working directory: " << cwd << endl;
    }

    if (!glfwInit())
    {
        cout << "Error initializing GLFW" << endl;
        return -1;
    }
    cout << "GLFW initialized successfully." << endl;

    // 设置OpenGL版本为4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "读取着色器文件示例", NULL, NULL);
    if (!window)
    {
        cout << "Error creating window" << endl;
        glfwTerminate();
        return -1;
    }
    cout << "Window created successfully." << endl;

    // 接下来绑定上下文
    glfwMakeContextCurrent(window);

    // 初始化GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        cout << "Error initializing GLEW: " << glewGetErrorString(err) << endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    cout << "GLEW initialized successfully." << endl;
    cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;

    init(window);

    // 进入主循环中
    cout << "Entering main loop." << endl;
    while (!glfwWindowShouldClose(window))
    {
        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteProgram(renderingprogram);
    glDeleteVertexArrays(numvaos, vao);

    glfwDestroyWindow(window);
    glfwTerminate();

    cout << "Program ended successfully." << endl;
    system("pause"); // 暂停控制台，防止窗口闪退
    return 0;
}