/*
 * 文件名：test.cpp
 * 功能：使用OpenGL和GLFW绘制一个可交互的彩色三角形
 *
 * 主要功能说明：
 * 1. 程序启动后在窗口中心绘制一个三角形，三角形的颜色为随机生成。
 * 2. 鼠标左键按下时，三角形颜色会随机变化。
 * 3. 鼠标右键按下时，三角形会变大，超过一定大小后重置为初始大小，并且三角形会旋转。
 * 4. 支持窗口关闭和资源释放。
 *
 * 主要模块说明：
 * - generate_vertex(radius)：根据当前半径和角度生成三角形顶点坐标。
 * - generate_color()：为三角形的每个顶点生成随机颜色。
 * - check_mouse(...)：鼠标回调函数，打印鼠标事件信息（实际逻辑在主循环中处理）。
 * - display(...)：负责清屏并绘制三角形。
 * - main()：程序入口，负责窗口和OpenGL初始化、主循环、事件处理和资源释放。
 *
 * 使用说明：
 * - 需要先安装GLEW和GLFW库，并正确配置开发环境。
 * - 编译运行后，点击窗口左键可变色，右键可变大并旋转。
 *
 * 适用范围：
 * - OpenGL学习入门、交互式图形编程实验。
 *
 * 作者：武汉大学国家网络安全学院
 * 日期：2025年5月
 */

// 绘制三角形
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

const float MY_PI = 3.1415926f;
// 定义全局颜色数组
float colors[9];
// 定义三角形的半径
float radius = 0.5f;
// 定义三角形顶点的角度
float angle[3] = {0.0f, 2 * MY_PI / 3, 4 * MY_PI / 3};
// 生成三角形顶点
vector<float> generate_vertex(float radius)
{
    vector<float> vertex;
    for (int i = 0; i < 3; i++)
    {
        vertex.push_back(radius * cos(2 * MY_PI * angle[i] / 3));
        vertex.push_back(radius * sin(2 * MY_PI * angle[i] / 3));
        vertex.push_back(0.0f);
    }
    return vertex;
}
void generate_color()
{
    for (int i = 0; i < 9; i++)
    {
        colors[i] = rand() % 255 / 255.0f;
    }
}
// 鼠标回调函数
void check_mouse(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        cout << "left mouse button pressed,and the color will be changed" << endl;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        cout << "right mouse button pressed,and the size will be changed" << endl;
    }
}

// 绘制三角形
void display(GLFWwindow *window, const vector<float> &vertex)
{
    glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 3; i++)
    {
        glColor3f(colors[i * 3], colors[i * 3 + 1], colors[i * 3 + 2]);
        glVertex3f(vertex[i * 3], vertex[i * 3 + 1], vertex[i * 3 + 2]);
    }
    glEnd();

    glfwSwapBuffers(window);
    glfwPollEvents();
    glFlush();
}

int main()
{
    // 初始化glfw
    if (!glfwInit())
    {
        cout << "glfwInit error" << endl;
        return -1;
    }
    // 创建窗口
    GLFWwindow *window = glfwCreateWindow(800, 600, "Triangle", NULL, NULL);
    if (!window)
    {
        cout << "create window error" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 初始化GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cout << "glewInit error" << endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, 800, 600);

    // 设置鼠标回调函数
    glfwSetMouseButtonCallback(window, check_mouse);
    // 在外面设置三角形的顶点
    vector<float> vertex = generate_vertex(radius);
    // 生成随机颜色
    generate_color();
    while (!glfwWindowShouldClose(window))
    {
        display(window, vertex);
        // 检测鼠标 只有摁下左键 才继续刷新
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            generate_color();
        }
        else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            radius += 0.01f;
            vertex = generate_vertex(radius);
            // 检查边界
            if (radius > 1.0f)
            {
                radius = 0.5f;
            }
            // 改变角度
            for (int i = 0; i < 3; i++)
            {
                angle[i] += 0.02f;
                if (angle[i] > 2 * MY_PI)
                {
                    angle[i] -= 2 * MY_PI;
                }
            }
        }
        else
        {
            glfwPollEvents();
        }
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}