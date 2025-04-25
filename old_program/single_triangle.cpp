#include <GL/glut.h>
#include <iostream>
#include <vector>
using namespace std;

// 全局变量存储顶点数据
vector<float> triangleVertices;
vector<float> squareVertices;

// 初始化环境
void init(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 设置背景颜色为黑色
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0); // 设置正交投影范围
}

// 绘制三角形
void display_triangle()
{
    glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲区

    glBegin(GL_TRIANGLES); // 开始绘制三角形 1
    glColor3f(triangleVertices[2], triangleVertices[3], triangleVertices[4]);
    glVertex2f(triangleVertices[0], triangleVertices[1]);
    // 2
    glColor3f(triangleVertices[7], triangleVertices[8], triangleVertices[9]);
    glVertex2f(triangleVertices[5], triangleVertices[6]);
    // 3
    glColor3f(triangleVertices[12], triangleVertices[13], triangleVertices[14]);
    glVertex2f(triangleVertices[10], triangleVertices[11]);
    glEnd();

    glFlush(); // 强制执行所有 OpenGL 命令
}

// 绘制四边形（暂未实现）
void display_square()
{
    // 空实现
    // 清除颜色缓冲区
    glClear(GL_COLOR_BUFFER_BIT);
    // 开始绘制正方形 1
    glBegin(GL_LINE_LOOP);
    glColor3f(squareVertices[2], squareVertices[3], squareVertices[4]);
    glVertex2f(squareVertices[0], squareVertices[1]);
    // 2
    glColor3f(squareVertices[7], squareVertices[8], squareVertices[9]);
    glVertex2f(squareVertices[5], squareVertices[6]);
    // 3
    glColor3f(squareVertices[12], squareVertices[13], squareVertices[14]);
    glVertex2f(squareVertices[10], squareVertices[11]);
    // 4
    glColor3f(squareVertices[17], squareVertices[18], squareVertices[19]);
    glVertex2f(squareVertices[15], squareVertices[16]);
    glEnd();

    // 强制执行OpenGL的命令
    glFlush();
}

int main(int argc, char **argv)
{
    // 设置随机数种子
    srand(time(NULL));
    // 初始化 GLUT 库
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); // 设置显示模式
    glutInitWindowPosition(100, 200);            // 设置窗口位置
    glutInitWindowSize(400, 600);                // 设置窗口大小
    glutCreateWindow("Hello World!");            // 创建窗口并设置标题

    // 初始化环境
    init();
    int choice = 2;
    cout << "请选择要绘制的图形：" << endl;
    cout << "1. 三角形" << endl;
    cout << "2. 正方形" << endl;
    if (choice == 1)
    {
        // // 设置三角形顶点数据
        // 随机生成triangleVertices
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 5; j++)
            {
                triangleVertices.push_back((float)rand() / RAND_MAX * 2 - 1);
            }
        }

        // 设置显示回调函数
        glutDisplayFunc(display_triangle);

        // 进入主循环
        glutMainLoop();
    }
    else if (choice == 2)
    {
        // 随机生成正方形数据
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 5; j++)
            {
                squareVertices.push_back((float)rand() / RAND_MAX * 2 - 1);
            }
        }
        // 增加自定义的function
        glutDisplayFunc(display_square);

        // 进入主循环
        glutMainLoop();
    }
    // 回收所有资源
    triangleVertices.clear();
    squareVertices.clear();
    glutDestroyWindow(glutGetWindow());
    return 0;
}