#version 430 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aColor;
//输出到片段着色器中
out vec3 vertexColor;
void main()
{
    gl_Position=vec4(aPos,1.0);
    vertexColor=aColor; //将顶点颜色传递给片段着色器
}