#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
using namespace std;

int main()
{
    // 初始化 GLFW
    if (!glfwInit()) {
        cout << "Failed to initialize GLFW" << endl;
        return -1;
    }

    // 设置 OpenGL 版本和属性
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); // 使用兼容模式

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello Triangle with glBegin", NULL, NULL);
    if (window == nullptr) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 初始化 GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        cout << "Failed to initialize GLEW" << endl;
        glfwTerminate();
        return -1;
    }

    // 设置视口
    glViewport(0, 0, 800, 600);

    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        // 处理事件
        glfwPollEvents();

        // 清除颜色缓冲区
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 使用 glBegin 和 glEnd 绘制三角形
        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f); // 顶点颜色：红色
        glVertex3f(-0.5f, -0.5f, 0.0f); // 左下角
        glColor3f(0.0f, 1.0f, 0.0f); // 顶点颜色：绿色
        glVertex3f(0.5f, -0.5f, 0.0f); // 右下角
        glColor3f(0.0f, 0.0f, 1.0f); // 顶点颜色：蓝色
        glVertex3f(0.0f, 0.5f, 0.0f); // 顶部
        glEnd();

        // 交换缓冲区
        glfwSwapBuffers(window);
    }

    // 清理资源
    glfwTerminate();
    return 0;
}