#include <GL/glut.h>
#include <cmath>
#include <iostream>
using namespace std;

void init(void)
{
    glClearColor(0.5f, 0.2f, 0.8f, 0.0f); // 设置背景颜色
    glMatrixMode(GL_PROJECTION);          // 设置投影矩阵
    glLoadIdentity();                     // 重置投影矩阵
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);     // 设置视口
    // 设置正交投影范围
    glMatrixMode(GL_MODELVIEW); // 设置模型视图矩阵
    glLoadIdentity();           // 重置模型视图矩阵
}
// 绘制圆形
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 0.0f, 0.0f); // 设置颜色为红色
    int num_segments = 100;      // 分割数
    float radius = 0.5f;         // 半径
    for (int i = 0; i <= num_segments; i++)
    {
        float theta = 2.0f * 3.1415926f * float(i) / float(num_segments); // 计算角度
        float x = radius * cosf(theta);                                   // 计算x坐标
        float y = radius * sinf(theta);                                   // 计算y坐标
        glVertex2f(x, y);                                                 // 设置顶点
    }
    glEnd();   // 结束绘制
    glFlush(); // 强制执行OpenGL的命令
}
int main(int argc, char **argv)
{
    // 初始化glfw
    glutInit(&argc, argv);
    // 设置显示模式
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    // 设置窗口的位置和大小
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 800);
    // 创建窗口并设置标题
    glutCreateWindow("MY SECOND OPENGL PROGRAM");
    // 初始化环境
    init();
    // 设置并显示回调函数
    glutDisplayFunc(display);
    // 进入主循环、
    glutMainLoop();
    // 进入主循环
    glutMainLoop();
    return 0;
}