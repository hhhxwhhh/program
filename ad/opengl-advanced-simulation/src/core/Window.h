#ifndef WINDOW_H
#define WINDOW_H

#include <GLFW/glfw3.h>

class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    void create();
    void close();
    bool shouldClose();
    void pollEvents();
    void swapBuffers();

    GLFWwindow* getWindow() const;

private:
    int width;
    int height;
    const char* title;
    GLFWwindow* window;
};

#endif // WINDOW_H