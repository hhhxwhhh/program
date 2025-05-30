#include <iostream>
#include <string>

// 手动定义 GLuint64 类型
#ifndef __gl_h_
typedef unsigned long long GLuint64;
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>
using namespace std;

int main()
{
    // 设置随机数种子
    srand(static_cast<unsigned int>(time(NULL)));

    // 初始化 GLFW
    if (!glfwInit()) {
        cout << "Failed to initialize GLFW" << endl;
        return -1;
    }

    // 设置窗口属性
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello World", nullptr, nullptr);
    if (window == nullptr) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }

    // 设置当前窗口上下文
    glfwMakeContextCurrent(window);

    // 设置垂直同步
    glfwSwapInterval(1);

    // 初始化 GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        cout << "Failed to initialize GLEW" << endl;
        glfwTerminate();
        return -1;
    }

    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // 处理事件

        // 设置背景颜色
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 交换缓冲区
        glfwSwapBuffers(window);
    }

    // 清理资源
    glfwTerminate();
    return 0;
}