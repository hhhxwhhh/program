#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

const unsigned int SCR_WIDTH = 2400;
const unsigned int SCR_HEIGHT = 1600;
const unsigned int INSTANCE_COUNT = 100000;
// 顶点着色器


const char *vertexShaderSource = R"(
#version 330 core 
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 instanceOffset;
layout (location = 2) in float instanceRotation;

out vec3 vertexColor;

uniform mat4 projection;
uniform mat4 view;
uniform float time;

// 平移函数
mat4 translate(mat4 matrix, vec3 offset) {
    matrix[3].x += offset.x;
    matrix[3].y += offset.y;
    matrix[3].z += offset.z;
    return matrix;
}

// 绕Y轴旋转函数（修正版）
mat4 rotateY(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    
    // 构造旋转矩阵
    mat4 rotationMatrix = mat4(1.0);
    rotationMatrix[0][0] = c;
    rotationMatrix[0][2] = s;
    rotationMatrix[2][0] = -s;
    rotationMatrix[2][2] = c;
    
    return rotationMatrix;
}

void main()
{
    // 计算旋转角度
    float rotation = instanceRotation + time;
    
    // 创建基础变换矩阵
    mat4 transform = mat4(1.0);
    
    // 应用平移
    transform = translate(transform, instanceOffset);
    
    // 应用旋转
    mat4 rotationMatrix = rotateY(rotation);
    transform = transform * rotationMatrix;

    gl_Position = projection * view * transform * vec4(aPos, 1.0);

    // 颜色计算
    vertexColor = vec3(0.5 + 0.5 * sin(rotation), 
                       0.5 + 0.5 * cos(rotation), 
                       0.5 + 0.5 * sin(rotation * 2.0));
}
)";
// 片段着色器

const char *fragmentShaderSource = R"(
#version 330 core 
out vec4 FragColor;
in vec3 vertexColor;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}

)";

float cubeVertices[] =
    {
        // 前面
        -0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,

        // 后面
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        // 左面
        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,

        // 右面
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,

        // 上面
        -0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f,

        // 下面
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f
    };

unsigned int createShaderProgram(const char *vertexShaderSource,const char *fragmentShaderSource)
{
    // 创建顶点着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    //检查编译错误
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader,512,NULL,infoLog);
        cerr<<"顶点着色器编译错误:"<<infoLog<<endl;
    }

    // 创建片段着色器   
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader,512,NULL,infoLog);
        cerr<<"片段着色器编译错误:"<<infoLog<<endl;
    }

    //创建着色器程序
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram,vertexShader);
    glAttachShader(shaderProgram,fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram,GL_LINK_STATUS,&success);
    if(!success)
    {
        glGetProgramInfoLog(shaderProgram,512,NULL,infoLog);
        cerr<<"着色器程序链接错误:"<<infoLog<<endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;

}
// 处理输入
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS)
        glfwSetWindowShouldClose(window,true);

}
// 窗口大小回调函数
void framebuffer_size_callback(GLFWwindow *window,int width,int height)
{
    glViewport(0,0,width,height);
}

int main()
{
    // 初始化GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "rotation example", NULL, NULL);
    if (!window) {
        cerr << "窗口创建失败" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // 初始化GLEW
    glewInit();
    if(glewInit()!=GLEW_OK)
    {
        cerr<<"GLEW初始化失败"<<endl;
        return -1;
    }

    //启动深度测试
    glEnable(GL_DEPTH_TEST);

    //创建着色器程序
    unsigned int shaderProgram = createShaderProgram(vertexShaderSource,fragmentShaderSource);

    //创建VAO和VBO
    unsigned int cubevao, cubevbo;
    glGenVertexArrays(1, &cubevao);
    glGenBuffers(1, &cubevbo);

    glBindVertexArray(cubevao);
    glBindBuffer(GL_ARRAY_BUFFER, cubevbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    //位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    //生成实例化数据
    srand(static_cast<unsigned int>(time(0)));
    vector<glm::vec3> translations(INSTANCE_COUNT);
    vector<float> rotations(INSTANCE_COUNT);

    float scale = 50.f;
    for (unsigned int i = 0; i < INSTANCE_COUNT;i++)
    {
        //随机位置
        translations[i].x = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f) * scale;
        translations[i].y = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f) * scale;
        translations[i].z = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f) * scale;
        

        //随机的旋转程序
        rotations[i]=(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 360.0f;
    }
    //实例化vbo
    unsigned int instancevbo[2];
    glGenBuffers(2, instancevbo);

    glBindBuffer(GL_ARRAY_BUFFER, instancevbo[0]);
    glBufferData(GL_ARRAY_BUFFER, translations.size() * sizeof(glm::vec3), translations.data(), GL_STATIC_DRAW);
    //旋转vbo
    glBindBuffer(GL_ARRAY_BUFFER, instancevbo[1]);
    glBufferData(GL_ARRAY_BUFFER, rotations.size() * sizeof(float), rotations.data(), GL_STATIC_DRAW);

    //为vao设置属性
    glBindVertexArray(cubevao);
    glBindBuffer(GL_ARRAY_BUFFER, instancevbo[0]);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1,1);

    glBindBuffer(GL_ARRAY_BUFFER, instancevbo[1]);//旋转属性
    glVertexAttribPointer(2,1,GL_FLOAT,GL_FALSE,sizeof(float),(void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2,1);

    //设置变换矩阵
    glm::mat4 projection=glm::perspective(glm::radians(45.0f),(float)SCR_WIDTH/(float)SCR_HEIGHT,0.1f,2500.0f);

    //进入主循环开始渲染
    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //设置相机位置
        float time=glfwGetTime();
        float camerax = sin(time * 0.5f) * 100.0f;
        float cameray = cos(time * 0.5f) * 100.0f;
        float cameraz = -100.0f;
        glm::mat4 view=glm::lookAt(glm::vec3(camerax,cameray,cameraz),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f));
        
        glUseProgram(shaderProgram);

        //设置变换矩阵
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"projection"),1,GL_FALSE,glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"view"),1,GL_FALSE,glm::value_ptr(view));
        glUniform1f(glGetUniformLocation(shaderProgram,"time"),time);

        //绘制立方体
        glBindVertexArray(cubevao);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, INSTANCE_COUNT);

        glfwSwapBuffers(window);
        glfwPollEvents();
        cout << "THE END" << endl;
    }
    
    glDeleteVertexArrays(1, &cubevao);
    glDeleteBuffers(1, &cubevbo);
    glDeleteBuffers(2, instancevbo);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    system("pause");
    return 0;
}


