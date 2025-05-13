#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
using namespace std;

void init(GLFWwindow *window)
{
}
void display(GLFWwindow *window, double time)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
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