#include <iostream>
#include "core/Application.h"

int main() {
    Application app;

    if (!app.initialize()) {
        std::cerr << "Failed to initialize the application!" << std::endl;
        return -1;
    }

    app.run();
    return 0;
}