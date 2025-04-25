#include <GL/glew.h>
#include <iostream>

int main()
{
    // 定义一个 GLchar 数组
    const GLchar *testString = "这是一个 GLchar 测试字符串";

    // 输出 GLchar 字符串
    std::cout << "GLchar 测试字符串: " << testString << std::endl;

    // 定义一个 GLchar 数组用于输入
    GLchar inputBuffer[256];

    // 提示用户输入
    std::cout << "请输入一段文字 (GLchar 类型): ";
    std::cin.getline(inputBuffer, 256);

    // 输出用户输入的内容
    std::cout << "您输入的内容是: " << inputBuffer << std::endl;
    system("pause"); // 暂停程序以查看输出
    return 0;
}