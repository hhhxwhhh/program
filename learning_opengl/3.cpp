#include <iostream>
#include <vector>
#include <algorithm>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
using namespace std;

//绘制顶点着色器
const char *vertexshadersource = R"(
#version 330 core
layout(location=0) in vec3 aPos;
void main()
{
    gl_Position=vec4(aPos.x,aPos.y,aPos.z,1.0f);
}
)";

//片段着色器源码 - 修复：移除分号
const char *fragmentshadersource = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor=vec4(1.0f,0.5f,0.2f,1.0f);//设置颜色为橙色
}
)";

int main()
{
    //初始化GLFW
    if(!glfwInit())
    {
        cout<<"Failed to initialize GLFW"<<endl;
        return -1;
    }
    
    //设置opengl版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    
    //创建窗口
    GLFWwindow *window = glfwCreateWindow(800,600,"Modern OpenGL Triangle",NULL,NULL);
    if(window==NULL)
    {
        cout<<"Failed to create GLFW window"<<endl;
        glfwTerminate();
        return -1;
    }
    
    //设置窗口上下文为当前线程的主上下文
    glfwMakeContextCurrent(window);
    
    //在确定上下文以后，才能初始化glew
    glewExperimental = GL_TRUE;
    if(glewInit()!=GLEW_OK)
    {
        cout<<"Failed to initialize GLEW"<<endl;
        return -1;
    }
    
    //设置观察的视口
    glViewport(0,0,800,600);
    
    //定义顶点数据
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
    };
    
    //创建VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    //创建VBO
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    //将定义的顶点数据传递给vbo 在gpu中存储数据
    //第一个参数是目标缓冲的类型，第二个参数是缓冲的大小，第三个参数是数据，第四个参数是数据的使用方式
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //配置顶点属性
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void *)0);
    //启用顶点属性
    glEnableVertexAttribArray(0);
    
    //创建并编译着色器
    unsigned int vertexshader=glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexshader, 1, &vertexshadersource, NULL);
    glCompileShader(vertexshader);
    
    //检查编译是否成功
    int success;
    char infolog[512];
    glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexshader, 512, NULL, infolog);
        cout<<"ERROR::SHADER::VERTEX::COMPILATION_FAILED"<<endl;
        cout<<infolog<<endl;
    }
    
    //创建并编译片段着色器
    unsigned int fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentshader, 1, &fragmentshadersource, NULL);
    glCompileShader(fragmentshader);
    
    //检查编译是否成功
    glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentshader, 512, NULL, infolog);
        cout<<"ERROR::SHADER::FRAGMENT::COMPILATION_FAILED"<<endl;
        cout<<infolog<<endl;
    }
    
    //创建程序对象并且连接
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexshader);
    glAttachShader(shaderProgram, fragmentshader);
    glLinkProgram(shaderProgram);
    
    //检查链接错误
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infolog);
        cout<<"ERROR::SHADER::PROGRAM::LINKING_FAILED"<<endl;
        cout<<infolog<<endl;
    }
    
    //已经链接到程序里面了 现在要删除着色器
    glDeleteShader(vertexshader);
    glDeleteShader(fragmentshader);
    
    cout<<"OpenGL Triangle Rendering Started Successfully!"<<endl;
    cout<<"Window Size: 800x600"<<endl;
    cout<<"OpenGL Version: 3.3 Core Profile"<<endl;
    
    //渲染循环
    while(!glfwWindowShouldClose(window))
    {
       //处理输入事件
       glfwPollEvents();
       
       //设置背景并清除颜色缓冲
       glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
       glClear(GL_COLOR_BUFFER_BIT);
       
       //使用着色器程序
       glUseProgram(shaderProgram);
       
       //绑定vao 绘制三角形
       glBindVertexArray(vao);
       glDrawArrays(GL_TRIANGLES, 0, 3);
       
       //交换前后缓冲区（双缓冲技术，避免闪烁）
       glfwSwapBuffers(window);
    }
    
    cout<<"Cleaning up resources..."<<endl;
    
    //清理资源
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);
    
    //终止GLFW
    glfwTerminate();
    
    cout<<"Program terminated successfully!"<<endl;
    
    return 0;
}