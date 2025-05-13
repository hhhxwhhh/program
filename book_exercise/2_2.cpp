#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
using namespace std;
#define numvaos 1
GLuint vao[numvaos];
GLuint renderingprogram;
// create shader program
GLuint createshaderprogram()
{
    // 这个事硬编码的顶点着色器
    const char *vshadersource =
        "#version 430 core\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n" // 绘制一个点在原点
        "}\n";

    const char *fshadersource =
        "#version 430 core\n"
        "out vec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    fragColor = vec4(1.0, 0.0, 0.0, 1.0);\n" // 红色输出
        "}\n";
    // 创建着色器对象
    GLuint vertexshader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    // attach code with the shader object
    glShaderSource(vertexshader, 1, &vshadersource, NULL);
    glShaderSource(fragmentshader, 1, &fshadersource, NULL);
    // compile the shader
    glCompileShader(vertexshader);
    glCompileShader(fragmentshader);
    // create the program object
    GLuint program = glCreateProgram();
    // attach the shaders to the program
    glAttachShader(program, vertexshader);
    glAttachShader(program, fragmentshader);
    // link
    glLinkProgram(program);
    return program;
}
void init(GLFWwindow *window)
{
    renderingprogram = createshaderprogram();
    glGenVertexArrays(numvaos, vao);
    glBindVertexArray(vao[0]);
}
void display(GLFWwindow *window, double time)
{
    glUseProgram(renderingprogram);
    glPointSize(100.0);
    glDrawArrays(GL_POINTS, 0, 1);
}
int main(void)
{
    if (!glfwInit())
    {
        cout << "Error initializing GLFW" << endl;
        return -1;
    }
    // 这里不设定opengl的版本号
    GLFWwindow *window = glfwCreateWindow(800, 600, "hello my first time", NULL, NULL);
    if (!window)
    {
        cout << "Error creating window" << endl;
        glfwTerminate();
        return -1;
    }
    // 接下来绑定上下文
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK)
    {
        cout << "Error initializing GLEW" << endl;
        return -1;
    }
    init(window);
    // 进入主循环中
    while (!glfwWindowShouldClose(window))
    {
        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}