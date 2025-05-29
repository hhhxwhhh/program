#version 430 core
in vec3 vertexColor; //从顶点着色器接收颜色
out vec4 FragColor; //输出到屏幕的颜色
void main()
{
    FragColor=vec4(vertexColor, 1.0); //设置输出颜色为传入的顶点颜色，alpha值为1.0
}