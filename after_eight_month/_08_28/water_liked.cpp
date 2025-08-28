#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>

// 水流计算着色器
const char* waterComputeShaderSource = R"(
#version 430 core

struct WaterParticle {
    vec4 position;
    vec4 velocity;
    vec4 color;
    float life;
    float maxLife;
    float density;
};

layout(std430, binding = 0) buffer ParticleBuffer {
    WaterParticle particles[];
};

uniform float deltaTime;
uniform float time;
uniform vec3 emitterPosition;
uniform vec3 gravity;

void main() {
    uint index = gl_GlobalInvocationID.x;
    
    if (index >= gl_NumWorkGroups.x * gl_WorkGroupSize.x) return;
    
    WaterParticle p = particles[index];
    
    // 更新生命周期
    p.life -= deltaTime;
    
    if (p.life <= 0.0) {
        // 重置死亡粒子
        p.position = vec4(emitterPosition + vec3(
            (float(randivec3(gl_GlobalInvocationID.xyy).x) / float(0x7fffffff)) * 2.0 - 1.0,
            (float(randivec3(gl_GlobalInvocationID.xyx).y) / float(0x7fffffff)) * 2.0 - 1.0,
            (float(randivec3(gl_GlobalInvocationID.yxy).z) / float(0x7fffffff)) * 2.0 - 1.0
        ) * 0.5, 1.0);
        
        p.velocity = vec4(0.0, 0.0, 0.0, 0.0);
        p.color = vec4(0.2, 0.4, 1.0, 0.7);
        p.life = p.maxLife = 3.0 + (float(randivec3(gl_GlobalInvocationID.xxx).x) / float(0x7fffffff)) * 2.0;
        p.density = 1.0;
    } else {
        // 更新存活粒子
        // 应用重力
        p.velocity.xyz += gravity * deltaTime;
        
        // 水流相互作用
        vec3 force = vec3(0.0);
        for (int i = 0; i < 10; i++) {
            uint otherIndex = (index + i * 100) % (gl_NumWorkGroups.x * gl_WorkGroupSize.x);
            if (otherIndex != index) {
                WaterParticle other = particles[otherIndex];
                vec3 diff = p.position.xyz - other.position.xyz;
                float dist = length(diff);
                if (dist > 0.0 && dist < 1.0) {
                    float strength = 0.5 / (dist + 0.1);
                    force -= normalize(diff) * strength * other.density;
                }
            }
        }
        
        // 应用流体动力学力
        p.velocity.xyz += force * deltaTime;
        
        // 限制最大速度
        float maxSpeed = 10.0;
        if (length(p.velocity.xyz) > maxSpeed) {
            p.velocity.xyz = normalize(p.velocity.xyz) * maxSpeed;
        }
        
        // 更新位置
        p.position.xyz += p.velocity.xyz * deltaTime;
        
        // 碰撞检测 - 地面
        if (p.position.y < -5.0) {
            p.position.y = -5.0;
            p.velocity.y *= -0.3; // 弹跳
            p.velocity.x *= 0.8;  // 摩擦
            p.velocity.z *= 0.8;  // 摩擦
        }
        
        // 碰撞检测 - 墙壁
        if (p.position.x < -10.0 || p.position.x > 10.0) {
            p.velocity.x *= -0.5;
            p.position.x = clamp(p.position.x, -10.0, 10.0);
        }
        if (p.position.z < -10.0 || p.position.z > 10.0) {
            p.velocity.z *= -0.5;
            p.position.z = clamp(p.position.z, -10.0, 10.0);
        }
        
        // 颜色随速度变化
        float speed = length(p.velocity.xyz);
        p.color = vec4(0.2 + speed * 0.02, 0.4 + speed * 0.01, 0.8 + speed * 0.03, 0.6 + speed * 0.02);
        
        // 透明度随生命周期变化
        float lifeRatio = p.life / p.maxLife;
        p.color.a = lifeRatio < 0.2 ? lifeRatio * 5.0 : 1.0;
    }
    
    particles[index] = p;
}
)";

// 水流渲染顶点着色器
const char* waterVertexShaderSource = R"(
#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    TexCoord = aTexCoord;
    gl_Position = vec4(aPos, 1.0);
}
)";

// 水流几何着色器
const char* waterGeometryShaderSource = R"(
#version 430 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec2 TexCoord[];

out vec2 oTexCoord;
out vec4 oColor;
out vec2 oPosition;
flat out vec3 oCenter;

struct WaterParticle {
    vec4 position;
    vec4 velocity;
    vec4 color;
    float life;
    float maxLife;
    float density;
};

layout(std430, binding = 0) buffer ParticleBuffer {
    WaterParticle particles[];
};

uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main() {
    uint index = uint(gl_PrimitiveIDIn);
    WaterParticle p = particles[index];
    
    if (p.life <= 0.0) return;
    
    vec3 pos = p.position.xyz;
    vec4 color = p.color;
    float size = 0.1 + 0.1 * sin(time * 3.0 + index);
    
    // 相机面向的四边形
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(vec3(view[0][0], view[1][0], view[2][0]), up));
    up = normalize(cross(right, vec3(view[0][0], view[1][0], view[2][0])));
    
    vec4 center = projection * view * vec4(pos, 1.0);
    
    oColor = color;
    oCenter = center.xy / center.w;
    
    // 生成四边形
    vec4 offset;
    
    // 左下
    offset = center + vec4(-right * size - up * size, 0.0);
    gl_Position = offset;
    oTexCoord = vec2(0.0, 0.0);
    oPosition = (offset.xy / offset.w) - oCenter;
    EmitVertex();
    
    // 右下
    offset = center + vec4(right * size - up * size, 0.0);
    gl_Position = offset;
    oTexCoord = vec2(1.0, 0.0);
    oPosition = (offset.xy / offset.w) - oCenter;
    EmitVertex();
    
    // 左上
    offset = center + vec4(-right * size + up * size, 0.0);
    gl_Position = offset;
    oTexCoord = vec2(0.0, 1.0);
    oPosition = (offset.xy / offset.w) - oCenter;
    EmitVertex();
    
    // 右上
    offset = center + vec4(right * size + up * size, 0.0);
    gl_Position = offset;
    oTexCoord = vec2(1.0, 1.0);
    oPosition = (offset.xy / offset.w) - oCenter;
    EmitVertex();
    
    EndPrimitive();
}
)";

// 水流片段着色器
const char* waterFragmentShaderSource = R"(
#version 430 core

in vec2 oTexCoord;
in vec4 oColor;
in vec2 oPosition;
flat in vec3 oCenter;

out vec4 FragColor;

void main() {
    float dist = length(oTexCoord - 0.5);
    if (dist > 0.5) discard;
    
    // 水滴形状
    float alpha = 1.0 - smoothstep(0.4, 0.5, dist);
    
    // 添加水面反射效果
    vec3 color = oColor.rgb * alpha;
    
    // 添加波纹效果
    float wave = sin(length(oPosition) * 20.0 - gl_FragCoord.x * 0.1) * 0.1;
    color += vec3(wave, wave, wave);
    
    // 添加高光
    float specular = exp(-length(oPosition) * 10.0) * 0.5;
    color += vec3(specular * 0.8, specular, specular * 1.2);
    
    FragColor = vec4(color, alpha * oColor.a);
}
)";

// 地面顶点着色器
const char* groundVertexShaderSource = R"(
#version 430 core

layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// 地面片段着色器
const char* groundFragmentShaderSource = R"(
#version 430 core

out vec4 FragColor;

void main() {
    FragColor = vec4(0.3, 0.5, 0.3, 1.0); // 绿色地面
}
)";

// 后处理顶点着色器
const char* postProcessVertexShaderSource = R"(
#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    TexCoord = aTexCoord;
    gl_Position = vec4(aPos, 1.0);
}
)";

// 后处理片段着色器 - 水效果增强
const char* postProcessFragmentShaderSource = R"(
#version 430 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D scene;
uniform float time;

void main() {
    vec2 texCoord = TexCoord;
    
    // 基础颜色
    vec3 color = texture(scene, texCoord).rgb;
    
    // 水面波纹效果
    vec2 rippleCoord = texCoord + vec2(
        sin(texCoord.y * 10.0 + time) * 0.005,
        cos(texCoord.x * 10.0 + time) * 0.005
    );
    
    vec3 rippleColor = texture(scene, rippleCoord).rgb;
    
    // 混合波纹效果
    color = mix(color, rippleColor, 0.3);
    
    // 蓝色调增强水感
    color.b += 0.1;
    
    // 伽马校正
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}
)";

// 着色器编译函数
unsigned int compileShader(const char* source, GLenum shaderType) {
    unsigned int shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cout << "Shader compilation failed:\n" << infoLog << std::endl;
    }
    
    return shader;
}

// 着色器程序链接函数
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource, const char* geometrySource = nullptr) {
    unsigned int vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    
    unsigned int geometryShader = 0;
    if (geometrySource) {
        geometryShader = compileShader(geometrySource, GL_GEOMETRY_SHADER);
    }
    
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    if (geometrySource) {
        glAttachShader(program, geometryShader);
    }
    glLinkProgram(program);
    
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        std::cout << "Program linking failed:\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometrySource) {
        glDeleteShader(geometryShader);
    }
    
    return program;
}

// 水流粒子系统类
class WaterParticleSystem {
public:
    static const int PARTICLE_COUNT = 8000;
    
    struct WaterParticle {
        glm::vec4 position;
        glm::vec4 velocity;
        glm::vec4 color;
        float life;
        float maxLife;
        float density;
    };
    
    unsigned int computeProgram;
    unsigned int renderProgram;
    unsigned int ssbo;
    WaterParticle particles[PARTICLE_COUNT];
    
    WaterParticleSystem() {
        // 初始化粒子
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posDist(-1.0f, 1.0f);
        std::uniform_real_distribution<float> lifeDist(2.0f, 4.0f);
        
        for (int i = 0; i < PARTICLE_COUNT; ++i) {
            particles[i].position = glm::vec4(0.0f, 5.0f, 0.0f, 1.0f);
            particles[i].velocity = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            particles[i].color = glm::vec4(0.2f, 0.4f, 1.0f, 0.7f);
            particles[i].life = lifeDist(gen);
            particles[i].maxLife = particles[i].life;
            particles[i].density = 1.0f;
        }
        
        // 创建着色器程序
        unsigned int computeShader = compileShader(waterComputeShaderSource, GL_COMPUTE_SHADER);
        computeProgram = glCreateProgram();
        glAttachShader(computeProgram, computeShader);
        glLinkProgram(computeProgram);
        glDeleteShader(computeShader);
        
        renderProgram = createShaderProgram(waterVertexShaderSource, waterFragmentShaderSource, waterGeometryShaderSource);
        
        // 创建SSBO
        glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(WaterParticle) * PARTICLE_COUNT, particles, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
    
    void update(float deltaTime, float time) {
        // 更新粒子
        glUseProgram(computeProgram);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
        glUniform1f(glGetUniformLocation(computeProgram, "deltaTime"), deltaTime);
        glUniform1f(glGetUniformLocation(computeProgram, "time"), time);
        glUniform3f(glGetUniformLocation(computeProgram, "emitterPosition"), 0.0f, 5.0f, 0.0f);
        glUniform3f(glGetUniformLocation(computeProgram, "gravity"), 0.0f, -9.8f, 0.0f);
        glDispatchCompute((PARTICLE_COUNT + 255) / 256, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
    
    void render(const glm::mat4& view, const glm::mat4& projection, float time) {
        glUseProgram(renderProgram);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
        glUniformMatrix4fv(glGetUniformLocation(renderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(renderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(glGetUniformLocation(renderProgram, "time"), time);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        
        // 绘制粒子
        glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);
        
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
};

// 地面渲染类
class GroundRenderer {
public:
    unsigned int shaderProgram;
    unsigned int VAO, VBO, EBO;
    
    GroundRenderer() {
        // 地面顶点数据
        float vertices[] = {
            // 位置
            -10.0f, -5.0f, -10.0f,
             10.0f, -5.0f, -10.0f,
             10.0f, -5.0f,  10.0f,
            -10.0f, -5.0f,  10.0f
        };
        
        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };
        
        // 创建着色器程序
        shaderProgram = createShaderProgram(groundVertexShaderSource, groundFragmentShaderSource);
        
        // 创建缓冲对象
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindVertexArray(0);
    }
    
    void render(const glm::mat4& view, const glm::mat4& projection) {
        glUseProgram(shaderProgram);
        
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

// 全屏四边形数据
float quadVertices[] = {
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

int main() {
    // 初始化GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    
    GLFWwindow* window = glfwCreateWindow(1200, 900, "Water Flow Simulation", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // 初始化GLEW
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    // OpenGL设置
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glDepthFunc(GL_LESS);
    
    // 创建水流粒子系统
    WaterParticleSystem waterSystem;
    
    // 创建地面渲染器
    GroundRenderer groundRenderer;
    
    // 创建后处理着色器程序
    unsigned int postProcessProgram = createShaderProgram(postProcessVertexShaderSource, postProcessFragmentShaderSource);
    
    // 创建帧缓冲对象
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    // 创建颜色附件纹理
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1200, 900, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    
    // 创建渲染缓冲对象作为深度缓冲
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1200, 900);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // 创建全屏四边形VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    // 相机设置
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 20.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    
    float lastTime = 0.0f;
    
    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        // 处理输入
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        // 渲染到自定义帧缓冲
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 设置相机
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1200.0f / 900.0f, 0.1f, 100.0f);
        
        // 渲染地面
        groundRenderer.render(view, projection);
        
        // 更新和渲染水流
        waterSystem.update(deltaTime, currentTime);
        waterSystem.render(view, projection, currentTime);
        
        // 渲染到屏幕
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(postProcessProgram);
        glBindVertexArray(quadVAO);
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glUniform1f(glGetUniformLocation(postProcessProgram, "time"), currentTime);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_DEPTH_TEST);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // 清理资源
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &textureColorbuffer);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteProgram(postProcessProgram);
    
    glfwTerminate();
    return 0;
}