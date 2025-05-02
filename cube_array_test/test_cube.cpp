#include <GL/glut.h>
#include <iostream>
using namespace std;
// 定义旋转角度
float angle = 0.0f;
void init()
{
    glClearColor(0.2f, 0.3f, 0.5f, 0.4f);                // 设置背景颜色为黑色
    glEnable(GL_DEPTH_TEST);                             // 启动深度测试
    glEnable(GL_LIGHTING);                               // 启用光照
    glEnable(GL_LIGHT0);                                 // 启用光源0
    glEnable(GL_COLOR_MATERIAL);                         // 启用材质
    GLfloat light_position[] = {1.0f, 1.0f, 1.0f, 0.0f}; // 设置光源位置
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);   // 设置光源位置
}
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -6.0f);
    glRotatef(angle, 0.75f, 0.55f, 0.3f); // 绕y轴旋转

    // 绘制cube
    glBegin(GL_QUADS);
    // 前面
    glColor3f(1.0f, 0.0f, 0.0f); // 红色
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    // 后面
    glColor3f(0.0f, 1.0f, 0.0f); // 绿色
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    // 上面
    glColor3f(0.0f, 0.0f, 1.0f); // 蓝色
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    // 下面
    glColor3f(1.0f, 1.0f, 0.0f); // 黄色
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    // 左面
    glColor3f(1.0f, 0.0f, 1.0f); // 紫色
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    // 右面
    glColor3f(0.0f, 1.0f, 1.0f); // 青色
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glEnd();

    glutSwapBuffers();  // 交换缓冲区
    glFlush();          // 强制执行OpenGL命令
    angle += 0.1f;      // 增加旋转角度
    if (angle > 360.0f) // 限制旋转角度在0-360之间
    {
        angle -= 360.0f;
    }
}
void reshape(int w, int h)
{
    if (h == 0)
    {
        h = 1;
    }
    float aspect = (float)w / (float)h;
    glViewport(0, 0, w, h);                      // 设置视口
    glMatrixMode(GL_PROJECTION);                 // 设置投影矩阵
    glLoadIdentity();                            // 加载单位矩阵
    gluPerspective(45.0f, aspect, 0.1f, 100.0f); // 设置透视投影
    glMatrixMode(GL_MODELVIEW);                  // 设置模型视图矩阵
}
int main(int argc, char **argv)
{
    glutInit(&argc, argv); // 初始化GLUT库
    // 设置显示模式为双缓冲和RGB颜
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);     // 设置窗口大小
    glutInitWindowPosition(100, 100); // 设置窗口位置
    glutCreateWindow("3D Cube");      // 创建窗口
    init();                           // 初始化GLUT库
    glutDisplayFunc(display);
    glutIdleFunc(display);    // 设置空闲回调函数
    glutReshapeFunc(reshape); // 设置重塑回调函数
    glutMainLoop();           // 进入主循环
    return 0;
}