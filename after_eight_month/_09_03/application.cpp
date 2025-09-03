#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"

#include <iostream>
#include <cmath>
using namespace std;
//自适应的函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char *path);

//定义屏幕的参数常量
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//开始引入shader
const char *vertexShaderSource = R"(
#version 330 core; 
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;  

void main()
{
    gl_Position=projection*view*model*vec4(aPos,1.0);
    FragPos=vec3(model*vec4(aPos,1.0f));
    Normal=mat3(transpose(inverse(model)))*aNormal;
    TexCoord=aTexCoord;
}
)";
