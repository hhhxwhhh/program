#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <fstream>

// 纹理对象
unsigned int texture;
float rotationAngle = 0.0f;
using namespace std;
// 检查文件是否存在
bool fileExists(const char* filename) {
    std::ifstream file(filename);
    return file.good();
}

void init() {
    // 启用纹理
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST); // 启用深度测试
    
    // 设置清除颜色
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // 生成并绑定纹理
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // 尝试多种路径加载图像
    const char* imagePaths[] = {
        "1.jpg",              // 当前目录
        "./1.jpg",            // 明确当前目录
        "../_08_31/1.jpg",    // 原始路径
        "./after_eight_month/_08_31/1.jpg",  // 从根目录开始的路径
        "after_eight_month/_08_31/1.jpg"     // 从根目录开始的路径（无./）
    };
    
    unsigned char *data = nullptr;
    int width, height, nrChannels;
    bool loaded = false;
    
    // 先检查文件是否存在
    for (int i = 0; i < 5; i++) {
        bool exists = fileExists(imagePaths[i]);
        std::cout << "文件 " << imagePaths[i] << (exists ? " 存在" : " 不存在") << std::endl;
        cout<<"fileExists"<<imagePaths[i]<<endl;
    }
    
    // 尝试不同的路径
    for (int i = 0; i < 5; i++) {
        std::cout << "尝试加载图片: " << imagePaths[i] << std::endl;
        data = stbi_load(imagePaths[i], &width, &height, &nrChannels, 0);
        if (data) {
            std::cout << "successs load: " << imagePaths[i] << std::endl;
            std::cout << "图片尺寸: " << width << "x" << height << ", 通道数: " << nrChannels << std::endl;
            loaded = true;
            break;
        } else {
            std::cout << "从路径加载失败: " << imagePaths[i] << std::endl;
            const char* error = stbi_failure_reason();
            if (error != nullptr) 
            {
                std::cout << "错误原因: " << stbi_failure_reason() << std::endl;
            }
        }
    }
    
    if (loaded) {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        else {
            std::cout << "不支持的通道数: " << nrChannels << std::endl;
            loaded = false;
        }
        
        if (loaded) {
            // 生成纹理
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);
            
            std::cout << "成功创建纹理: " << width << "x" << height << " 像素, " << nrChannels << " 通道" << std::endl;
        }
    }
    
    if (!loaded) {
        std::cout << "纹理加载失败! 使用默认颜色纹理" << std::endl;
        
        // 创建一个简单的彩色纹理作为替代
        unsigned char simpleTexture[] = {
            255, 0, 0,     0, 255, 0,     0, 0, 255,     255, 255, 0,  // 红 绿 蓝 黄
            255, 0, 255,   0, 255, 255,   128, 128, 128,  255, 255, 255 // 品红 青 灰 白
        };
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, simpleTexture);
    }
    
    // 释放图像内存
    if (data) {
        stbi_image_free(data);
    }
}

void display() {
    // 清除颜色和深度缓冲区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 设置视图矩阵
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 5.0,  // 相机位置
              0.0, 0.0, 0.0,  // 目标位置
              0.0, 1.0, 0.0); // 上方向量
    
    // 绑定纹理
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // 应用旋转变换
    glRotatef(rotationAngle, 1.0f, 1.0f, 1.0f);
    
    // 绘制立方体的六个面
    glBegin(GL_QUADS);
        // 前面 (Z = 1.0)
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
        
        // 后面 (Z = -1.0)
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
        
        // 上面 (Y = 1.0)
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
        
        // 下面 (Y = -1.0)
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
        
        // 右面 (X = 1.0)
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
        
        // 左面 (X = -1.0)
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    glEnd();
    
    glutSwapBuffers();
}

void reshape(int width, int height) {
    // 设置视口
    glViewport(0, 0, width, height);
    
    // 设置投影矩阵
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 1.0, 100.0);
}

void idle() {
    // 更新旋转角度
    rotationAngle += 0.5f;
    if (rotationAngle > 360.0f) {
        rotationAngle -= 360.0f;
    }
    
    // 请求重绘
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    std::cout << "start" << std::endl;
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Cube Texture");
    
    init();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle); // 注册空闲回调函数以实现动画效果
    
    glutMainLoop();
    
    return 0;
}