cmake_minimum_required(VERSION 3.10)
project(opengl-advanced-simulation)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(src)

# Source files
file(GLOB_RECURSE SOURCES
    src/*.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries (if any)
# target_link_libraries(${PROJECT_NAME} <your_libraries>)