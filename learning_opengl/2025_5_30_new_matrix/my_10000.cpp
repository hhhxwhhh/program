#include <iostream>
#include <vector>
#include <random>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

const int CUBE_COUNT = 100000;

//顶点着色器
const char *vertexShaderSource = R"(
#version 430 core
//输入属性（从CPU传入的数据）
layout(location=0) in vec3 aPos; // 顶点位置
layout(location=1) in vec3 aInstancePos; // 实例位置
layout(location=2) in vec3 aInstanceColor; // 实例颜色
layout(location=3) in float aInstanceScale; // 实例缩放
layout(location=4) in float aInstanceSpeed; // 实例动画速度

//输出属性（传递给片段着色器的数据）
out vec3 vertexColor; // 传递给片段着色器的颜色

//uniform变量（从CPU传入的矩阵和时间）
uniform mat4 view; // 视图矩阵
uniform mat4 projection; // 投影矩阵
uniform float time; // 时间变量，用于动画

void main()
{
    //给予时间和速度计算旋转角度
    float rotationAngle=time*aInstanceSpeed;

    //创建旋转矩阵 一y轴旋转
    mat4 rotation= mat4(1.0);
    rotation[0][0]=cos(rotationAngle);
    rotation[0][2]=sin(rotationAngle);
    rotation[2][0]=-sin(rotationAngle);
    rotation[2][2]=cos(rotationAngle);

    //应用缩放
    vec3 scaledPos=aPos*aInstanceScale;

    //应用旋转
    vec4 rotatedPos=rotation*vec4(scaledPos,1.0);

    //应用位置偏移
    vec4 worldPos=vec4(rotatedPos.xyz+aInstancePos,1.0);

    //应用视图和投影矩阵
    gl_Position=projection*view*worldPos;

    //计算动态颜色亮度
    float colorIntensity=0.7+0.3*sin(time*aInstanceSpeed+length(aInstancePos));

    //将颜色插值到顶点颜色
    vertexColor=aInstanceColor*colorIntensity;
}

)";

//片段着色器
const char *fragmentShaderSource = R"(
#version 430 core
in  vec3 vertexColor; // 从顶点着色器传入的颜色

out vec4 FragColor;

void main()
{
    FragColor=vec4(vertexColor,1.0);
}
)";

//编译着色器
unsigned int compileShader(const char* source,GLenum type)
{
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);
    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        cout << "Shader compilation error:" << infoLog << endl;
    }
    return id;
}

//创建着色器程序
unsigned int createShaderProgram()
{
    unsigned int vertexShader= compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    if(vertexShader == 0 || fragmentShader == 0)
    {
        return 0;
    }
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout<< "Program linking error:" << infoLog << endl;
        return 0;
    }

    //删除着色器对象（已经链接到程序中，不再需要）
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    cout << "Shader program created successfully!" << endl;
    return shaderProgram;
}
//处理输入
void processInput(GLFWwindow* window)
{
    if(glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}
// 窗口大小回调
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{

    //初始化阶段
    if(!glfwInit())
    {
        cout << "Failed to initialize GLFW!" << endl;
        return -1;
    }

    // 设置OpenGL版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 启用多重采样抗锯齿

    // 创建窗口
    GLFWwindow* window=glfwCreateWindow(1600,1200,"🎲10,000 动画立方体",NULL,NULL);

    if(!window)
    {
        cout << "Failed to create GLFW window!" << endl;
        glfwTerminate();
        return -1;
    }

    // 设置窗口上下文
    glfwMakeContextCurrent(window);
    // 设置窗口大小回调
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //初始化GLEW
    glewExperimental = GL_TRUE; 
    if(glewInit()!=GLEW_OK)
    {
        cout << "Failed to initialize GLEW!" << endl;
        return -1;
    }
    
    // 打印OpenGL版本 GLSL版本
    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    //启动渲染设置
    glEnable(GL_DEPTH_TEST); // 启用深度测试
    glDepthFunc(GL_LESS); // 深度测试函数
    glEnable(GL_CULL_FACE); // 启用面剔除
    glEnable(GL_MULTISAMPLE); // 启用多重采样抗锯齿
    glCullFace(GL_BACK); // 剔除背面
    glFrontFace(GL_CCW); // 设置前面为逆时针

    //定义几何数据
    float vertices[] = 
    {
        // 前面的4个顶点
        -0.5f, -0.5f,  0.5f,  // 0: 左下前
         0.5f, -0.5f,  0.5f,  // 1: 右下前
         0.5f,  0.5f,  0.5f,  // 2: 右上前
        -0.5f,  0.5f,  0.5f,  // 3: 左上前
        // 后面的4个顶点
        -0.5f, -0.5f, -0.5f,  // 4: 左下后
         0.5f, -0.5f, -0.5f,  // 5: 右下后
         0.5f,  0.5f, -0.5f,  // 6: 右上后
        -0.5f,  0.5f, -0.5f   // 7: 左上后
    };

    unsigned int indices[] =
        {
            // 前面
            0, 1, 2, 2, 3, 0,
            // 后面
            4, 5, 6, 6, 7, 4,
            7, 3, 0, 0, 4, 7, // 左面
            1, 5, 6, 6, 2, 1, // 右面
            3, 2, 6, 6, 7, 3, // 上面
            0, 1, 5, 5, 4, 0  // 下面
        };

    // 随机生成实例数据
    cout<<"Random generate "<<CUBE_COUNT<<"numbers!"<<endl;

    // 创建随机数生成器
    vector<vec3> instancePositions(CUBE_COUNT);
    vector<vec3> instanceColors(CUBE_COUNT);
    vector<float> instanceScales(CUBE_COUNT);
    vector<float> instanceSpeeds(CUBE_COUNT);

    //设置随机数生成器
    random_device rd;
    mt19937 gen(rd());// 随机数引擎

    //定义随机数分布
    uniform_real_distribution<float>posRange(-50.0f, 50.0f); // 位置范围
    uniform_real_distribution<float>colorRange(0.3f, 1.0f); // 颜色范围
    uniform_real_distribution<float>scaleRange(0.1f, 1.0f); // 缩放范围
    uniform_real_distribution<float>speedRange(0.1f, 2.0f); // 速度范围

    //随机生成每个立方体的属性
    for (int i = 0; i < CUBE_COUNT;i++)
    {
        instancePositions[i]=vec3(posRange(gen),posRange(gen),posRange(gen));
        instanceColors[i]=vec3(colorRange(gen),colorRange(gen),colorRange(gen));
        instanceScales[i]=scaleRange(gen);
        instanceSpeeds[i]=speedRange(gen);
    }
    cout << "All the random values have been generated!" << endl;

    //GPU缓冲区设置

    //创建顶点数组对象
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    //绑定用于记录所有的设置
    glBindVertexArray(VAO);

    //绑定顶点缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //绑定索引缓冲区
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //解析顶点的属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // 生成实例化数据缓冲区
    unsigned int instanceVBO[4]; // 位置、颜色、缩放、速度

    // 创建实例位置缓冲区
    glGenBuffers(1, &instanceVBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * CUBE_COUNT, &instancePositions[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1); // 每个实例一个属性

    // 创建实例颜色缓冲区
    glGenBuffers(1, &instanceVBO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * CUBE_COUNT, &instanceColors[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1); // 每个实例一个属性

    // 创建实例缩放缓冲区
    glGenBuffers(1, &instanceVBO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * CUBE_COUNT, &instanceScales[0], GL_STATIC_DRAW);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1); // 每个实例一个属性

    // 创建实例速度缓冲区
    glGenBuffers(1, &instanceVBO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * CUBE_COUNT, &instanceSpeeds[0], GL_STATIC_DRAW);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1); // 每个实例一个属性

    cout << "VAO and instance buffers created successfully!" << endl;

    //创建着色器程序
    unsigned int  shaderProgram = createShaderProgram();
    //uniform变量位置
    GLint viewloc=glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint timeLoc = glGetUniformLocation(shaderProgram, "time");
    if(viewloc == -1 || projLoc == -1 || timeLoc == -1)
    {
        cout << "Failed to get uniform locations!" << endl;
        return -1;
    }
    if(shaderProgram == 0)
    {
        cout << "Failed to create shader program!" << endl;
        return -1;
    }
    cout << "Shader program created successfully!" << endl;

    //主渲染循环
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        processInput(window);
        //获取当前时间（用于动画）
        float currentTime = static_cast<float>(glfwGetTime());
        //设置背景色
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        //创建环绕相机（相机绕着立方体群转圈）
        float radius = 80.0f;
        float cam_x= radius * sin(currentTime * 0.2f);
        float cam_z = radius * cos(currentTime * 0.2f);
        float cam_y=sin(currentTime * 0.1f) * 20.0f; // 相机高度随时间变化

        //创建视图矩阵
        mat4 view=lookAt(vec3(cam_x,cam_y,cam_z),// 相机位置
        vec3(0.0f,0.0f,0.0f),// 相机目标
        vec3(0.0f,1.0f,0.0f));// 相机上方向

        //创建投影矩阵
        mat4 projection=perspective(
        radians(45.0f),// 视角
        1200.0f/900.0f,// 宽高比
        0.1f,// 近裁剪面
        100.0f// 远裁剪面
        );

        //使用着色器程序
        glUseProgram(shaderProgram);
        //设置uniform变量
        glUniformMatrix4fv(viewloc,1,GL_FALSE,value_ptr(view));
        glUniformMatrix4fv(projLoc,1,GL_FALSE,value_ptr(projection));
        glUniform1f(timeLoc,currentTime);

        //实例化渲染
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, CUBE_COUNT); // 绘制所有立方体

        //交换缓冲区
        glfwSwapBuffers(window);
    }
    //清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();

    cout << "Program terminated!" << endl;


    return 0;
}