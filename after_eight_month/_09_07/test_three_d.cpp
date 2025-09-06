#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

// 顶点着色器
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";
// 片段着色器
const char *fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{

    // 环境光
    vec3 ambient = 0.1 * lightColor;
    ambient=objectColor*ambient;
    // 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    diffuse=objectColor*diffuse;
    // 镜面光
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * lightColor;
    specular=objectColor*specular;
    // 最终颜色
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
)";


//确定贝塞尔曲线的控制点
glm::vec3 controlPoints[4][4] = {
     {glm::vec3(-1.5f, 1.5f, 0.0f), glm::vec3(-0.5f, 1.5f, 1.0f), glm::vec3(0.5f, 1.5f, 1.0f), glm::vec3(1.5f, 1.5f, 0.0f)},
    {glm::vec3(-1.5f, 0.5f, 1.0f), glm::vec3(-0.5f, 0.5f, 2.0f), glm::vec3(0.5f, 0.5f, 2.0f), glm::vec3(1.5f, 0.5f, 1.0f)},
    {glm::vec3(-1.5f, -0.5f, 1.0f), glm::vec3(-0.5f, -0.5f, 2.0f), glm::vec3(0.5f, -0.5f, 2.0f), glm::vec3(1.5f, -0.5f, 1.0f)},
    {glm::vec3(-1.5f, -1.5f, 0.0f), glm::vec3(-0.5f, -1.5f, 1.0f), glm::vec3(0.5f, -1.5f, 1.0f), glm::vec3(1.5f, -1.5f, 0.0f)}
};

// 定义贝塞尔的基函数
float bernstein(int i,float t)
{
    switch (i)
    {
        case 0:
            return (1 - t) * (1 - t) * (1 - t);
        case 1:
            return 3 * t * (1 - t) * (1 - t);
        case 2:
            return 3 * t * t * (1 - t);
        case 3:
            return t * t * t;
        default:
            return 0;
    }
}

//计算贝塞尔函数的点
glm::vec3 bezierSurface(float u,float v)
{
    glm::vec3 result(0.0f);
    for (int i = 0; i < 4;i++)
    {
        for(int j=0;j<4;j++)
        {
            result+=controlPoints[i][j]*bernstein(i,u)*bernstein(j,v);
        }
    }
    return result;
}
//计算贝塞尔曲面的切线向量
glm::vec3 bezierSurfaceDerivateU(float u,float v)
{
    glm::vec3 result(0.0f);
    for (int i = 0; i < 4;i++)
    {
        for (int j = 0; j < 4;j++)
        {
            float du = 0;
            switch(i)
            {
                case 0:
                {
                    du=-3*(1-u)*(1-u);
                    break;
                }
                case 1:
                {
                    du=3*(1-u)*(1-u)-6*u*(1-u);
                    break;
                }
                case 2:
                {
                    du=6*u*(1-u)-3*u*u;
                    break;
                }
                case 3:
                {
                    du=3*u*u;
                    break;
                }
                default:
                {
                    break;
                }
            }
            result+=controlPoints[i][j]*du*bernstein(j,v);
        }
    }
    return result;
}
//
glm::vec3 bezierSurfaceDerivateV(float u,float v)
{
    glm::vec3 result(0.0f);
    for(int i=0;i<4;i++)
    {
        for (int j = 0; j < 4;j++)
        {
            float dv = 0;
            switch(j)
            {
                case 0:
                {
                    dv=-3*(1-v)*(1-v);
                    break;
                }
                case 1:
                {
                    dv=3*(1-v)*(1-v)-6*v*(1-v);
                    break;
                }
                case 2:
                {
                    dv=6*v*(1-v)-3*v*v;
                    break;
                }
                case 3:
                {
                    dv=3*v*v;
                    break;
                }
                default:
                {
                    break;
                }
            }
            result+=controlPoints[i][j]*bernstein(i,u)*dv;
        }
    }
    return result;
}

//生成·贝塞尔曲面的顶点数据
void generateBezierSurfaceVertices(vector<float>& vertices,int resolution=30)
{
    for (int i = 0;i<resolution;i++)
    {
        for (int j = 0;j<resolution;j++)
        {
            float u1 = (float)i / (resolution - 1);
            float v1 = (float)j / (resolution - 1);
            float u2 = (float)(i + 1) / (resolution - 1);
            float v2 = (float)(j + 1) / (resolution - 1);
            
            // 第一个三角形
            glm::vec3 p1 = bezierSurface(u1, v1);
            glm::vec3 p2 = bezierSurface(u2, v1);
            glm::vec3 p3 = bezierSurface(u2, v2);
            
            // 计算法向量
            glm::vec3 du1 = bezierSurfaceDerivateU(u1, v1);
            glm::vec3 dv1 = bezierSurfaceDerivateV(u1, v1);
            glm::vec3 normal1 = glm::normalize(glm::cross(du1, dv1));
            
            glm::vec3 du2 = bezierSurfaceDerivateU(u2, v1);
            glm::vec3 dv2 = bezierSurfaceDerivateV(u2, v1);
            glm::vec3 normal2 = glm::normalize(glm::cross(du2, dv2));
            
            glm::vec3 du3 = bezierSurfaceDerivateU(u2, v2);
            glm::vec3 dv3 = bezierSurfaceDerivateV(u2, v2);
            glm::vec3 normal3 = glm::normalize(glm::cross(du3, dv3));
            
            // 添加顶点
            vertices.insert(vertices.end(), {p1.x, p1.y, p1.z, normal1.x, normal1.y, normal1.z});
            vertices.insert(vertices.end(), {p2.x, p2.y, p2.z, normal2.x, normal2.y, normal2.z});
            vertices.insert(vertices.end(), {p3.x, p3.y, p3.z, normal3.x, normal3.y, normal3.z});
            
            // 第二个三角形
            glm::vec3 p4 = bezierSurface(u1, v1);
            glm::vec3 p5 = bezierSurface(u2, v2);
            glm::vec3 p6 = bezierSurface(u1, v2);
            
            glm::vec3 du4 = bezierSurfaceDerivateU(u1, v1);
            glm::vec3 dv4 = bezierSurfaceDerivateV(u1, v1);
            glm::vec3 normal4 = glm::normalize(glm::cross(du4, dv4));
            
            glm::vec3 du5 = bezierSurfaceDerivateU(u2, v2);
            glm::vec3 dv5 = bezierSurfaceDerivateV(u2, v2);
            glm::vec3 normal5 = glm::normalize(glm::cross(du5, dv5));
            
            glm::vec3 du6 = bezierSurfaceDerivateU(u1, v2);
            glm::vec3 dv6 = bezierSurfaceDerivateV(u1, v2);
            glm::vec3 normal6 = glm::normalize(glm::cross(du6, dv6));
            
            // 添加顶点
            vertices.insert(vertices.end(), {p4.x, p4.y, p4.z, normal4.x, normal4.y, normal4.z});
            vertices.insert(vertices.end(), {p5.x, p5.y, p5.z, normal5.x, normal5.y, normal5.z});
            vertices.insert(vertices.end(), {p6.x, p6.y, p6.z, normal6.x, normal6.y, normal6.z});
        }
    }
}
//编译着色器
unsigned int compileShader(unsigned int type,const char* source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << endl;
    }
    return shader;
}
//创建着色器程序
unsigned int createShader(const char* vertexShader,const char* fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vShader = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fShader = compileShader(GL_FRAGMENT_SHADER, fragmentShader);
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        cout << "ERROR::SHADER::LINK_FAILED\n" << infoLog << endl;
    }
    return program;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if(window == NULL)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    //初始化glew
    if(glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize GLEW" << endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    //创建着色器程序
    unsigned int shaderProgram = createShader(vertexShaderSource, fragmentShaderSource);
    vector<float> vertices;
    generateBezierSurfaceVertices(vertices,50);
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    //设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    //设置法线属性指针
    glEnableVertexAttribArray(1);

    glm::mat4 projection=glm::perspective(glm::radians(45.0f), (float)800.0f / (float)600.0f, 0.1f, 100.0f);
    glm::mat4 view=glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    while(!glfwWindowShouldClose(window))
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }
        //
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glm::mat4 model = glm::mat4(1.0f);
        model=glm::rotate(model,glm::radians(-30.0f),glm::vec3(1.0f,0.0f,0.0f));
        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        //设置光照
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 1.2f, 1.0f, 2.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), 0.0f, 0.0f, 5.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.5f, 0.8f, 1.0f);

        //绘制贝塞尔曲面
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}