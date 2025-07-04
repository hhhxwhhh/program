#ifndef APPLICATION_H
#define APPLICATION_H

#include "Window.h"

class Application {
public:
    Application();
    ~Application();

    void init();
    void run();
    void cleanup();

private:
    Window* window;
};

#endif // APPLICATION_H