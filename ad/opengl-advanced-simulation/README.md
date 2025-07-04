# OpenGL Advanced Simulation

This project is an advanced simulation using OpenGL, designed to create realistic rendering and physics interactions. The structure of the project is organized into several modules, each responsible for different aspects of the simulation.

## Project Structure

- **src/**: Contains the source code for the application.
  - **main.cpp**: Entry point of the application, responsible for initializing and running the main loop.
  - **core/**: Core application logic.
    - **Application.cpp**: Implements the main logic of the application.
    - **Application.h**: Declares the Application class and its public methods.
    - **Window.cpp**: Implements window creation and management.
    - **Window.h**: Declares the Window class and its public methods.
  - **renderer/**: Handles rendering of objects.
    - **Renderer.cpp**: Implements the rendering logic.
    - **Renderer.h**: Declares the Renderer class and its public methods.
    - **Shader.cpp**: Implements shader loading and compilation.
    - **Shader.h**: Declares the Shader class and its public methods.
    - **Texture.cpp**: Implements texture loading and binding.
    - **Texture.h**: Declares the Texture class and its public methods.
    - **VertexArray.cpp**: Implements vertex array management.
    - **VertexArray.h**: Declares the VertexArray class and its public methods.
    - **VertexBuffer.cpp**: Implements vertex buffer management.
    - **VertexBuffer.h**: Declares the VertexBuffer class and its public methods.
  - **physics/**: Contains physics simulation logic.
    - **PhysicsEngine.cpp**: Implements the physics engine logic.
    - **PhysicsEngine.h**: Declares the PhysicsEngine class and its public methods.
    - **RigidBody.cpp**: Implements rigid body properties and behaviors.
    - **RigidBody.h**: Declares the RigidBody class and its public methods.
  - **objects/**: Contains definitions for various 3D objects.
    - **Sphere.cpp**: Implements sphere geometry and rendering.
    - **Sphere.h**: Declares the Sphere class and its public methods.
    - **Cube.cpp**: Implements cube geometry and rendering.
    - **Cube.h**: Declares the Cube class and its public methods.
    - **Plane.cpp**: Implements plane geometry and rendering.
    - **Plane.h**: Declares the Plane class and its public methods.
  - **lighting/**: Handles lighting in the scene.
    - **Light.cpp**: Implements basic light properties and behaviors.
    - **Light.h**: Declares the Light class and its public methods.
    - **DirectionalLight.cpp**: Implements directional light characteristics.
    - **DirectionalLight.h**: Declares the DirectionalLight class and its public methods.
    - **PointLight.cpp**: Implements point light characteristics.
    - **PointLight.h**: Declares the PointLight class and its public methods.
  - **camera/**: Manages the camera view and projection.
    - **Camera.cpp**: Implements camera perspective and projection.
    - **Camera.h**: Declares the Camera class and its public methods.
  - **utils/**: Contains utility functions and classes.
    - **Math.h**: Declares mathematical utility functions.
    - **FileLoader.cpp**: Implements file loading logic.
    - **FileLoader.h**: Declares the FileLoader class and its public methods.

- **shaders/**: Contains shader files for rendering.
  - **vertex/**: Vertex shaders.
    - **basic.vert**: Basic vertex shader.
    - **phong.vert**: Phong shading vertex shader.
    - **pbr.vert**: Physically based rendering vertex shader.
  - **fragment/**: Fragment shaders.
    - **basic.frag**: Basic fragment shader.
    - **phong.frag**: Phong shading fragment shader.
    - **pbr.frag**: Physically based rendering fragment shader.

- **textures/**: Directory for texture files.

- **models/**: Directory for 3D model files.

- **CMakeLists.txt**: CMake build configuration file.

## Getting Started

To build and run the project, follow these steps:

1. Clone the repository.
2. Navigate to the project directory.
3. Create a build directory and navigate into it:
   ```
   mkdir build
   cd build
   ```
4. Run CMake to configure the project:
   ```
   cmake ..
   ```
5. Build the project:
   ```
   make
   ```
6. Run the application:
   ```
   ./opengl-advanced-simulation
   ```

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue for any suggestions or improvements.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.