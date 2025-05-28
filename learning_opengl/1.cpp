#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>

const int MAX_PARTICLES = 2000;
// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Particle structure
struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec4 color;
    float life;
    float size;

    Particle() : life(0.0f), size(0.0f) {}
};

// Shader sources for particles
const GLchar* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in float aSize;

out vec4 particleColor;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
    particleColor = aColor;
    gl_PointSize = aSize;
}
)";

const GLchar* fragmentShaderSource = R"(
#version 330 core
in vec4 particleColor;
out vec4 FragColor;

void main()
{
    // Create circular particles
    vec2 coord = gl_PointCoord - vec2(0.5);
    if(length(coord) > 0.5)
        discard;
    
    // Smooth fade at edges
    float alpha = 1.0 - length(coord) * 2.0;
    alpha = smoothstep(0.0, 1.0, alpha);
    
    FragColor = vec4(particleColor.rgb, particleColor.a * alpha);
}
)";

// Function to find an unused particle
int findUnusedParticle(const std::vector<Particle>& particles, int& lastUsedParticle) {
    for (int i = lastUsedParticle; i < MAX_PARTICLES; i++) {
        if (particles[i].life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }
    for (int i = 0; i < lastUsedParticle; i++) {
        if (particles[i].life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }
    return -1; // All particles are alive
}

void respawnParticle(Particle &particle, glm::vec2 offset = glm::vec2(0.0f, 0.0f)) {
    float random = ((rand() % 100) - 50) / 10.0f;
    float rColor = 0.5f + ((rand() % 100) / 100.0f);
    
    particle.position = glm::vec2(0.0f, -0.8f) + offset;
    particle.velocity = glm::vec2(random, 1.0f + (rand() % 50) / 10.0f);
    particle.color = glm::vec4(rColor, rColor * 0.5f, 0.2f, 1.0f);
    particle.life = 3.0f; // Particle lives for 3 seconds
    particle.size = 15.0f + (rand() % 10);
}

int main()
{
    // Initialize random seed
    srand(static_cast<unsigned int>(time(nullptr)));

    // Init GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Cool OpenGL Particles", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Init GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Enable point size control in shaders
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Build and compile our shader program
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // Link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Particle data
    std::vector<Particle> particles(MAX_PARTICLES);
    int lastUsedParticle = 0;
    
    // Create VAO and VBO for particles
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // Allocate space for particle data (position, color, size)
    // Each particle has: 2 floats (position) + 4 floats (color) + 1 float (size) = 7 floats
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 7 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    
    // Size attribute
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);

    // Get uniform locations
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    
    // Create orthographic projection matrix
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    // Timing
    float lastTime = 0.0f;
    
    // Particle vertex buffer data
    std::vector<GLfloat> particle_vertex_buffer_data(MAX_PARTICLES * 7);
    
    // Game loop
    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        glfwPollEvents();
        
        // Clear the screen
        glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Generate new particles
        int newParticles = (int)(deltaTime * 2000.0); // 2000 particles per second
        for (int i = 0; i < newParticles; i++) {
            int particleIndex = findUnusedParticle(particles, lastUsedParticle);
            if (particleIndex != -1) {
                respawnParticle(particles[particleIndex]);
            }
        }
        
        // Update all particles
        int particlesCount = 0;
        for (int i = 0; i < MAX_PARTICLES; i++) {
            Particle& p = particles[i];
            
            if (p.life > 0.0f) {
                // Decrease life
                p.life -= deltaTime;
                
                if (p.life > 0.0f) {
                    // Update particle physics
                    p.velocity.y -= 9.81f * deltaTime * 0.5f; // Gravity
                    p.position += p.velocity * deltaTime;
                    
                    // Update color (fade out)
                    p.color.a = p.life / 3.0f;
                    
                    // Update size based on life
                    p.size = 20.0f * (p.life / 3.0f);
                    
                    // Add particle to buffer
                    particle_vertex_buffer_data[particlesCount * 7 + 0] = p.position.x;
                    particle_vertex_buffer_data[particlesCount * 7 + 1] = p.position.y;
                    particle_vertex_buffer_data[particlesCount * 7 + 2] = p.color.r;
                    particle_vertex_buffer_data[particlesCount * 7 + 3] = p.color.g;
                    particle_vertex_buffer_data[particlesCount * 7 + 4] = p.color.b;
                    particle_vertex_buffer_data[particlesCount * 7 + 5] = p.color.a;
                    particle_vertex_buffer_data[particlesCount * 7 + 6] = p.size;
                    
                    particlesCount++;
                } else {
                    p.color.a = 0.0f;
                }
            }
        }
        
        // Use shader program
        glUseProgram(shaderProgram);
        
        // Pass projection matrix to shader
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        
        // Render particles
        if (particlesCount > 0) {
            glBindVertexArray(VAO);
            
            // Update VBO with new particle data
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, particlesCount * 7 * sizeof(GLfloat), particle_vertex_buffer_data.data());
            
            // Draw particles as points
            glDrawArrays(GL_POINTS, 0, particlesCount);
            
            glBindVertexArray(0);
        }
        
        glfwSwapBuffers(window);
    }
    
    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    
    glfwTerminate();
    return 0;
}