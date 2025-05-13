#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
using namespace std;

// 顶点和片元着色器源码
const char *vShaderSrc = R"(
    #version 410 core
    #version 410 core
    layout(location = 0) in vec3 pos;
    uniform float offsetX;
    void main() {
        gl_Position = vec4(pos.x + offsetX, pos.y, pos.z, 1.0);
    }
)";

const char *fShaderSrc = R"(
    #version 410 core
    out vec4 fragColor;
    void main() {
        fragColor = vec4(0.2, 0.6, 1.0, 1.0);
    }
)";

// 编译着色器
GLuint compileShader(GLenum type, const char *src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cout << "Shader Compile Error:\n"
             << infoLog << endl;
    }
    return shader;
}

// 创建着色器程序
GLuint createShaderProgram(const char *vSrc, const char *fSrc)
{
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vSrc);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fSrc);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        cout << "Program Link Error:\n"
             << infoLog << endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

// 初始化VAO和VBO
void initTriangle(GLuint &VAO, GLuint &VBO)
{
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f,
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
}

// 渲染
void render(GLuint shaderProgram, GLuint VAO, float offsetX)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    GLuint offset_x = glGetUniformLocation(shaderProgram, "offsetX");
    glUniform1f(offset_x, offsetX);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

int main()
{
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(640, 480, "hello Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        return -1;

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    GLuint VAO, VBO;
    initTriangle(VAO, VBO);
    GLuint shaderProgram = createShaderProgram(vShaderSrc, fShaderSrc);

    while (!glfwWindowShouldClose(window))
    {
        float time = static_cast<float>(glfwGetTime());
        float offsetX = 0.5f * sin(time); // 在[-0.5, 0.5]之间来回移动
        render(shaderProgram, VAO, offsetX);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    cout << "Bye~" << endl;
    return 0;
}