#include <GL/glut.h>
#include <cmath>
#include <iostream>
using namespace std;
// 绘制三角形
// 初始化环境
void init(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}
// 绘制的功能函数
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲区
    glBegin(GL_TRIANGLES);        // 开始绘制三角形
    glColor3f(1.0f, 0.0f, 0.0f);  // 设置颜色为红色
    glVertex2f(-0.5f, -0.5f);
    glColor3f(0.0f, 1.0f, 0.0f); // 设置颜色为绿色
    glVertex2f(0.5f, -0.5f);     // 设置第二个节点
    glColor3f(0.0f, 0.0f, 1.0f); // 设置颜色为蓝色
    glVertex2f(0.0f, 0.5f);      // 设置第三个节点
    glEnd();                     // 结束绘制
    glFlush();                   // 强制执行OpenGL的命令
}
// 显示正方形
void display_cube(void)
{
    glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲区
    glBegin(GL_QUADS);            // 开始绘制三角形
    glColor3f(1.0f, 0.0f, 0.0f);  // 设置颜色为红色
    glVertex2f(-0.5f, -0.5f);     // 设置第一个点的位置
    glColor3f(0.0f, 1.0f, 0.0f);  // 设置颜色为绿色
    glVertex2f(0.5f, -0.5f);      // 设置第二个节点
    glColor3f(0.0f, 0.0f, 1.0f);  // 设置颜色为蓝色
    glVertex2f(0.5f, 0.5f);       // 设置第三个点的位置
    glColor3f(0.0f, 1.0f, 1.0f);  // 设置颜色为蓝色
    glVertex2f(-0.5f, 0.5f);      // 设置第四个点的位置
    glEnd();                      // 结束绘制
    glFlush();                    // 强制执行OpenGL的   命令
}
int main(int argc, char **argv)
{
    // 初始化 GLUT 库
    glutInit(&argc, argv);
    // 设置显示模式
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    // 设置窗口的位置和大小
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(400, 400);
    // 创建窗口并设置标题
    glutCreateWindow("MY FIRST OPENGL PROGRAM");

    // 初始化环境
    init();
    // 设置显示回调函数
    glutDisplayFunc(display_cube);
    // 进入主循环
    glutMainLoop();

    return 0;
}