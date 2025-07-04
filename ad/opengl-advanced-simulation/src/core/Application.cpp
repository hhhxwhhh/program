#include "Application.h"
#include "Window.h"
#include "Renderer.h"

Application::Application(const std::string& title, int width, int height)
    : window(title, width, height), renderer() {
}

void Application::initialize() {
    window.initialize();
    renderer.initialize();
}

void Application::run() {
    while (!window.shouldClose()) {
        window.pollEvents();
        renderer.render();
        window.swapBuffers();
    }
}

void Application::cleanup() {
    renderer.cleanup();
    window.cleanup();
}