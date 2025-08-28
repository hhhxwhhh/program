#include <GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <string>
using namespace std;
// 顶点着色器

const char *vertexShaderSource = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;
layout(location=2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

uniform mat4 transform;
uniform int renderMode;


void main()
{
    gl_Position=transform*vec4(aPos,1.0f);
    if(renderMode==1)
    {
        ourColor=aColor;
    }
    else if(renderMode==2)
    {
        TexCoord=aTexCoord;
    }
}
)";
//片段着色器源代码
const char *fragmentShaderSource = R"(
#version 330 core 
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform int renderMode;// 不同的输入对应不同的渲染方式
uniform sampler2D ourTexture;

void main()
{
    if(renderMode==0)
    {
        //基于位置的颜色
        FragColor = vec4(abs(TexCoord.x), abs(TexCoord.y), 0.5, 1.0);
    }
    else if(renderMode==1)
    {
        //顶点颜色
        
        FragColor=vec4(ourColor,1.0);

    }
    else if(renderMode == 2)
    {
        // 纹理采样
        FragColor = texture(ourTexture, TexCoord);
    }
    else
    {
        // 默认颜色 白色
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }

}

)";

void checkCompileErrors(unsigned int shader, string type) 
{
    int success;
    char infoLog[1024];
    if(type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            cout<<"ERROR::SHADER_COMPILATION_ERROR of type:"<<type<<"\n"<<infoLog<<"\n";
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if(!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            cout<<"ERROR::PROGRAM_LINKING_ERROR of type:"<<type<<"\n"<<infoLog<<"\n";
        }
    }
}

//创建纹理程序
unsigned int loadTexture()
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    //创建简单的彩色纹理数据
    unsigned char data[4*4*3] = {
        255, 0, 0,   0, 255, 0,   0, 0, 255,   255, 255, 255,
        0, 255, 0,   255, 0, 0,   255, 255, 255, 0, 0, 255,
        0, 0, 255,   255, 255, 255, 255, 0, 0,   0, 255, 0,
        255, 255, 255, 0, 0, 255,   0, 255, 0,   255, 0, 0
    };
    glTexImage2D(GL_TEXTURE, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    //设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}

//设置渲染样式
enum RenderMode
{
    POSITION_BASED = 0,
    VERTEX_COLOR = 1,
    TEXTURE_BASED = 2
};

//主函数
int main()
{

    //初始化GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //创建窗口
    GLFWwindow *window = glfwCreateWindow(800, 600, "Particle System", NULL, NULL);
    if(window == NULL)
    {
        cout<<"Failed to create GLFW window"<<endl;
        glfwTerminate();
        return -1;
    }
    //创建上下文
    glfwMakeContextCurrent(window);

    //初始化GLEW
    glewInit();
    if(glewInit()!=GLEW_OK)
    {
        cout<<"Failed to initialize GLEW"<<endl;
        return -1;
    }
    
    //启动深度测试
    glEnable(GL_DEPTH_TEST);

        // 定义顶点数据（位置、颜色、纹理坐标）
    float vertices[] = {
        // 第一个三角形
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.5f, 1.0f,
         
        // 第二个三角形（用于纹理演示）
         0.0f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f,  0.5f, 1.0f
    };

    //创建VAO VBO
    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //设置位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //设置颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    

    //设置纹理坐标属性
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    

    //编译着色器
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");
    
    //链接着色器程序
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkCompileErrors(shaderProgram, "PROGRAM");

    //删除着色器对象
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //加载纹理
    unsigned int texture = loadTexture();

    //获取uniform位置
    int transformloc=glGetUniformLocation(shaderProgram,"transform");
    int rendermodeloc=glGetUniformLocation(shaderProgram,"renderMode");
    int textureloc=glGetUniformLocation(shaderProgram,"ourTexture");               
    
    //设置渲染模式
    RenderMode currentmode = POSITION_BASED;

    //循环渲染
    while(!glfwWindowShouldClose(window))
    {
        //处理输入
        if(glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS)
            glfwSetWindowShouldClose(window,true);
        

        //切换渲染模式
        if(glfwGetKey(window,GLFW_KEY_1)==GLFW_PRESS)
            currentmode=POSITION_BASED;
        if(glfwGetKey(window,GLFW_KEY_2)==GLFW_PRESS)
            currentmode=VERTEX_COLOR;
        if(glfwGetKey(window,GLFW_KEY_3)==GLFW_PRESS)
            currentmode=TEXTURE_BASED;


        //清除屏幕
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //使用着色器程序
        glUseProgram(shaderProgram);

        //设置渲染模式
        glUniform1i(rendermodeloc, currentmode);

        //设置对应的参数
        if(currentmode==TEXTURE_BASED)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            glUniform1i(textureloc, 0);
        }
        //创建旋转加平移矩阵
        float timeValue=glfwGetTime();
        float angle = 20.0f * timeValue;

        //旋转矩阵  
        float transform[16] = {
            cos(angle * 3.14159f / 180), -sin(angle * 3.14159f / 180), 0.0f, 0.0f,
            sin(angle * 3.14159f / 180), cos(angle * 3.14159f / 180), 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        //应用变换
        glUniformMatrix4fv(transformloc, 1, GL_FALSE, transform);

        //绘制三角形
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //交换缓冲区
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    //清理资源
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &texture);

    glfwTerminate();
    return 0;

}