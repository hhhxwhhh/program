#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

uniform float time;

void main()
{
    // 动态颜色效果
    vec3 color = vertexColor;
    color += 0.3 * sin(time * 3.0 + vertexColor.x * 10.0);
    FragColor = vec4(color, 1.0);
}