#include <GL/glut.h>
#include <cmath>
#include <iostream>
using namespace std;
// 全局变量
float radius = 0.2f;
const int num_segments = 100;
const float PI = 3.1415926f;
// 添加颜色变量
float circleColor[3] = {0.75f, 0.75f, 0.75f}; // 初始为灰色
// 初始化环境
void init(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 设置背景颜色为黑色
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0); // 设置正交投影范围
}
void display_circle(void)
{
    glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲区
    // 利用足够多的数量的三角形来绘制圆形
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(circleColor[0], circleColor[1], circleColor[2]); // 设置颜色为灰色
    for (int i = 0; i < num_segments; i++)
    {
        float theta = 2.0f * PI * float(i) / float(num_segments);
        float x = radius * cosf(theta);
        float y = radius * sinf(theta);
        glVertex2d(x, y); // 设置顶点
    }
    glEnd();   // 绘制结束
    glFlush(); // 强制执行OpenGL的命令
}
// 显示回调函数
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲区
    display_circle();             // 绘制圆形
    glutSwapBuffers();            // 交换缓冲区
}
void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_DOWN)
    {
        if (button == GLUT_LEFT_BUTTON)
        {
            radius = (float)rand() / RAND_MAX;
            cout << "鼠标左键点下 半径扩大为原来的二倍" << radius << endl;
            circleColor[0] = (float)rand() / RAND_MAX; // 随机颜色
            circleColor[1] = (float)rand() / RAND_MAX;
            circleColor[2] = (float)rand() / RAND_MAX;
        }
        else if (button == GLUT_RIGHT_BUTTON)
        {
            radius = (float)rand() / RAND_MAX;
            cout << "鼠标右键点下 半径缩小为原来的一半" << radius << endl;
        }
        else
        {
            // 变换颜色 随机数
            glColor3f((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
        }
        // 限制半径大小
        if (radius > 1.0f)
        {
            radius = 1.0f;
        }
        else if (radius < 0.01f)
        {
            radius = 0.01f;
        }
        // 重新绘制圆形
        glutPostRedisplay();
    }
}

// 键盘回调函数
void keyboard(unsigned char key, int x, int y)
{
    if (key == 'q' || key == 'Q')
    {
        exit(0); // 退出程序
    }
    else if (key == 'r' || key == 'R')
    {
        radius = 0.2f; // 重置半径
        cout << "重置半径为0.2" << endl;
        glutPostRedisplay(); // 重新绘制
    }
    else if (key == 'c' || key == 'C')
    {
        radius = 0.2f; // 重置半径
        cout << "重置半径为0.2" << endl;
        circleColor[0] = 0.75f;
        circleColor[1] = 0.75f;
        circleColor[2] = 0.75f; // 重置颜 色
        glutPostRedisplay();    // 重新绘制
    }
    else
    {
        cout << "无效的键盘输入" << endl;
    }
}

// 主程序
int main(int argc, char **argv)
{
    srand(time(NULL));                           // 设置随机数种子
    glutInit(&argc, argv);                       // 初始化GLUT库
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); // 设置显示模式为双缓冲和RGB颜色模式
    glutInitWindowSize(800, 800);                // 设置窗口大小为400x400像素
    glutInitWindowPosition(100, 100);            // 设置窗口位置为屏幕左上角的(100,100)像素
    glutCreateWindow("Circle");                  // 创建窗口并设置标题为"Circle"
    init();                                      // 初始化环境
    glutDisplayFunc(display);                    // 设置显示回调函数为display
    glutMouseFunc(mouse);                        // 设置鼠标回调函数为mouse
    glutKeyboardFunc(keyboard);                  // 设置键盘回调函数为keyboard
    glutMainLoop();                              // 进入主循环
    return 0;
}