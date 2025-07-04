#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <cmath>
#include <algorithm>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define PI 3.14159265359
using namespace std;
using namespace glm;

// 全局配置
const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 1200;
const int MAX_SPHERES = 50;

// 相机控制变量
float camera_yaw = -90.0f;
float camera_pitch = 0.0f;
float camera_fov = 45.0f;
vec3 camera_pos = vec3(0.0f, 5.0f, 15.0f);
vec3 camera_front = vec3(0.0f, 0.0f, -1.0f);
vec3 camera_up = vec3(0.0f, 1.0f, 0.0f);
bool first_mouse = true;
float last_x = WINDOW_WIDTH / 2.0f;
float last_y = WINDOW_HEIGHT / 2.0f;

// 时间变量
float delta_time = 0.0f;
float last_frame = 0.0f;

// 高级顶点着色器 - PBR + 物理仿真
const char* vertex_shader_source = R"(
#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

// 实例化属性
layout(location = 3) in vec3 aInstancePos;
layout(location = 4) in vec3 aInstanceVel;
layout(location = 5) in vec3 aInstanceColor;
layout(location = 6) in float aInstanceRadius;
layout(location = 7) in float aInstanceMass;

// 输出到片段着色器
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 InstanceColor;
out float InstanceRadius;
out vec3 ViewPos;

// Uniform变量
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform float time;

void main()
{
    // 动态缩放效果
    float pulse = sin(time * 2.0 + length(aInstancePos) * 0.1) * 0.1 + 1.0;
    vec3 scaledPos = aPos * aInstanceRadius * pulse;
    
    // 世界坐标计算
    vec4 worldPos = vec4(scaledPos + aInstancePos, 1.0);
    FragPos = worldPos.xyz;
    
    // 法线变换
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // 纹理坐标和颜色传递
    TexCoord = aTexCoord;
    InstanceColor = aInstanceColor;
    InstanceRadius = aInstanceRadius;
    ViewPos = viewPos;
    
    // 最终位置计算
    gl_Position = projection * view * worldPos;
}
)";

// 高级片段着色器 - PBR材质 + 多光源
const char* fragment_shader_source = R"(
#version 430 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 InstanceColor;
in float InstanceRadius;
in vec3 ViewPos;

out vec4 FragColor;

// 光源结构体
struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

// 材质属性
struct Material {
    float metallic;
    float roughness;
    float ao;
    vec3 albedo;
};

// Uniform变量
uniform DirectionalLight dirLight;
uniform PointLight pointLights[4];
uniform int numPointLights;
uniform float time;
uniform vec3 viewPos;

// PBR计算函数
vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, Material material);
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, Material material);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

void main()
{
    // 动态材质属性
    Material material;
    material.albedo = InstanceColor;
    material.metallic = 0.1 + sin(time + FragPos.x) * 0.3;
    material.roughness = 0.2 + cos(time + FragPos.y) * 0.3;
    material.ao = 1.0;
    
    // 基础向量计算
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(ViewPos - FragPos);
    
    // 方向光计算
    vec3 result = calculateDirectionalLight(dirLight, norm, viewDir, material);
    
    // 点光源计算
    for(int i = 0; i < numPointLights && i < 4; i++) {
        result += calculatePointLight(pointLights[i], norm, FragPos, viewDir, material);
    }
    
    // 发光效果
    float glow = sin(time * 3.0 + length(FragPos) * 0.2) * 0.1 + 0.9;
    result *= glow;
    
    // 色调映射和伽马校正
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}

// 方向光计算
vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, Material material)
{
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // PBR计算
    vec3 F0 = mix(vec3(0.04), material.albedo, material.metallic);
    
    float NDF = DistributionGGX(normal, halfwayDir, material.roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, material.roughness);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - material.metallic;
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    float NdotL = max(dot(normal, lightDir), 0.0);
    
    return (kD * material.albedo / 3.14159265 + specular) * light.diffuse * NdotL;
}

// 点光源计算
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, Material material)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // 距离衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // PBR计算
    vec3 F0 = mix(vec3(0.04), material.albedo, material.metallic);
    
    float NDF = DistributionGGX(normal, halfwayDir, material.roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, material.roughness);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - material.metallic;
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    float NdotL = max(dot(normal, lightDir), 0.0);
    
    vec3 result = (kD * material.albedo / 3.14159265 + specular) * light.diffuse * NdotL;
    return result * attenuation;
}

// PBR辅助函数
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265 * denom * denom;
    
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
)";

// 着色器编译函数
unsigned int compileShader(const char* source, GLenum type) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);
    
    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        cout << "Shader compilation error: " << infoLog << endl;
        return 0;
    }
    return id;
}

// 着色器程序创建
unsigned int createShaderProgram() {
    unsigned int vertexShader = compileShader(vertex_shader_source, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragment_shader_source, GL_FRAGMENT_SHADER);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        return 0;
    }
    
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "Shader program linking error: " << infoLog << endl;
        return 0;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    cout << "Advanced PBR shader program created successfully!" << endl;
    return shaderProgram;
}

// 物理球体类
class PhysicsSphere {
public:
    vec3 position;
    vec3 velocity;
    vec3 color;
    float radius;
    float mass;
    float restitution; // 弹性系数
    
    PhysicsSphere(vec3 pos, vec3 vel, vec3 col, float r, float m) 
        : position(pos), velocity(vel), color(col), radius(r), mass(m), restitution(0.8f) {}
    
    // 物理更新
    void update(float deltaTime) {
        // 重力作用
        vec3 gravity = vec3(0.0f, -9.81f, 0.0f);
        velocity += gravity * deltaTime;
        
        // 位置更新
        position += velocity * deltaTime;
        
        // 边界碰撞检测
        checkBoundaryCollision();
    }
    
    // 边界碰撞
    void checkBoundaryCollision() {
        float boundary = 10.0f;
        
        // X轴边界
        if (position.x + radius > boundary) {
            position.x = boundary - radius;
            velocity.x *= -restitution;
        } else if (position.x - radius < -boundary) {
            position.x = -boundary + radius;
            velocity.x *= -restitution;
        }
        
        // Y轴边界（地面）
        if (position.y - radius < -5.0f) {
            position.y = -5.0f + radius;
            velocity.y *= -restitution;
            
            // 地面摩擦
            velocity.x *= 0.95f;
            velocity.z *= 0.95f;
        }
        
        // Z轴边界
        if (position.z + radius > boundary) {
            position.z = boundary - radius;
            velocity.z *= -restitution;
        } else if (position.z - radius < -boundary) {
            position.z = -boundary + radius;
            velocity.z *= -restitution;
        }
    }
    
    // 球体碰撞检测
    bool checkCollision(PhysicsSphere& other) {
        float distance = length(position - other.position);
        return distance < (radius + other.radius);
    }
    
    // 碰撞响应
    void resolveCollision(PhysicsSphere& other) {
        vec3 normal = normalize(position - other.position);
        vec3 relativeVelocity = velocity - other.velocity;
        float separatingVelocity = dot(relativeVelocity, normal);
        
        if (separatingVelocity > 0) return; // 物体正在分离
        float restitutionCoeff = std::min(restitution, other.restitution);
        float newSepVelocity = -separatingVelocity * restitutionCoeff;
        float deltaVelocity = newSepVelocity - separatingVelocity;
        
        float totalInverseMass = 1.0f/mass + 1.0f/other.mass;
        float impulse = deltaVelocity / totalInverseMass;
        vec3 impulsePerMass = normal * impulse;
        
        velocity += impulsePerMass / mass;
        other.velocity -= impulsePerMass / other.mass;
        
        // 碰撞特效 - 颜色混合
        vec3 newColor = (color * mass + other.color * other.mass) / (mass + other.mass);
        color = mix(color, newColor, 0.1f);
        other.color = mix(other.color, newColor, 0.1f);
    }
};

// 输入处理函数
void processInput(GLFWwindow* window) {
    const float camera_speed = 10.0f * delta_time;
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
        
    // 相机移动
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_pos += camera_speed * camera_front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_pos -= camera_speed * camera_front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_pos -= normalize(cross(camera_front, camera_up)) * camera_speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_pos += normalize(cross(camera_front, camera_up)) * camera_speed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera_pos += camera_up * camera_speed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera_pos -= camera_up * camera_speed;
}

// 鼠标回调函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (first_mouse) {
        last_x = (float)xpos;
        last_y = (float)ypos;
        first_mouse = false;
    }
    
    float xoffset = (float)xpos - last_x;
    float yoffset = last_y - (float)ypos;
    last_x = (float)xpos;
    last_y = (float)ypos;
    
    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    
    camera_yaw += xoffset;
    camera_pitch += yoffset;
    
    if (camera_pitch > 89.0f) camera_pitch = 89.0f;
    if (camera_pitch < -89.0f) camera_pitch = -89.0f;
    
    // 更新相机方向
    vec3 front;
    front.x = cos(radians(camera_yaw)) * cos(radians(camera_pitch));
    front.y = sin(radians(camera_pitch));
    front.z = sin(radians(camera_yaw)) * cos(radians(camera_pitch));
    camera_front = normalize(front);
}

// 滚轮回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera_fov -= (float)yoffset;
    if (camera_fov < 1.0f) camera_fov = 1.0f;
    if (camera_fov > 45.0f) camera_fov = 45.0f;
}

// 窗口大小回调
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// 生成球体几何数据
vector<float> generateSphereVertices(float radius, int sectors, int stacks) {
    vector<float> vertices;
    
    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = PI / 2 - i * PI / stacks;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);
        
        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * 2 * PI / sectors;
            
            // 位置
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);
            
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            
            // 法线
            vertices.push_back(x / radius);
            vertices.push_back(y / radius);
            vertices.push_back(z / radius);
            
            // 纹理坐标
            vertices.push_back((float)j / sectors);
            vertices.push_back((float)i / stacks);
        }
    }
    
    return vertices;
}

// 生成球体索引
vector<unsigned int> generateSphereIndices(int sectors, int stacks) {
    vector<unsigned int> indices;
    
    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;
        
        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            
            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
    
    return indices;
}

// 主函数
int main() {
    cout << "Starting Advanced OpenGL Physics Simulation..." << endl;
    
    // 初始化GLFW
    if (!glfwInit()) {
        cout << "GLFW initialization failed!" << endl;
        return -1;
    }
    
    // OpenGL版本设置
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8); // 8倍MSAA
    
    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 
        "Advanced OpenGL Physics Simulation | PBR + Real-time Physics", NULL, NULL);
    
    if (!window) {
        cout << "Failed to create GLFW window!" << endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // 设置回调函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // 初始化GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        cout << "GLEW initialization failed!" << endl;
        return -1;
    }
    
    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
    
    // OpenGL设置
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // 创建着色器程序
    unsigned int shaderProgram = createShaderProgram();
    if (shaderProgram == 0) {
        cout << "Failed to create shader program!" << endl;
        return -1;
    }
    
    // 生成球体几何
    vector<float> sphereVertices = generateSphereVertices(1.0f, 30, 30);
    vector<unsigned int> sphereIndices = generateSphereIndices(30, 30);
    
    cout << "Generated sphere, vertex count: " << sphereVertices.size()/8 << endl;
    
    // 创建VAO, VBO, EBO
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), 
                 sphereVertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), 
                 sphereIndices.data(), GL_STATIC_DRAW);
    
    // 顶点属性设置
    // 位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 法线
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 纹理坐标
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // 创建物理球体
    vector<PhysicsSphere> spheres;
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> pos_dist(-8.0f, 8.0f);
    uniform_real_distribution<float> vel_dist(-5.0f, 5.0f);
    uniform_real_distribution<float> color_dist(0.3f, 1.0f);
    uniform_real_distribution<float> radius_dist(0.2f, 0.8f);
    uniform_real_distribution<float> mass_dist(1.0f, 5.0f);
    
    for (int i = 0; i < MAX_SPHERES; ++i) {
        vec3 pos = vec3(pos_dist(gen), pos_dist(gen) + 5.0f, pos_dist(gen));
        vec3 vel = vec3(vel_dist(gen), 0.0f, vel_dist(gen));
        vec3 color = vec3(color_dist(gen), color_dist(gen), color_dist(gen));
        float radius = radius_dist(gen);
        float mass = mass_dist(gen);
        
        spheres.emplace_back(pos, vel, color, radius, mass);
    }
    
    cout << "Created " << MAX_SPHERES << " physics spheres!" << endl;
    
    // 实例化数据缓冲区
    vector<vec3> instancePositions(MAX_SPHERES);
    vector<vec3> instanceVelocities(MAX_SPHERES);
    vector<vec3> instanceColors(MAX_SPHERES);
    vector<float> instanceRadii(MAX_SPHERES);
    vector<float> instanceMasses(MAX_SPHERES);
    
    unsigned int instanceVBOs[5];
    glGenBuffers(5, instanceVBOs);
    
    // 位置缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * MAX_SPHERES, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    
    // 速度缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * MAX_SPHERES, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);
    
    // 颜色缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * MAX_SPHERES, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1);
    
    // 半径缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MAX_SPHERES, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glEnableVertexAttribArray(6);
    glVertexAttribDivisor(6, 1);
    
    // 质量缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MAX_SPHERES, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glEnableVertexAttribArray(7);
    glVertexAttribDivisor(7, 1);
    
    cout << "Instanced buffers created successfully!" << endl;
    
    // 主渲染循环
    cout << "\nControls:" << endl;
    cout << "WASD - Move camera" << endl;
    cout << "Space/Shift - Ascend/Descend" << endl;
    cout << "Mouse - Look around" << endl;
    cout << "Scroll - Zoom" << endl;
    cout << "ESC - Exit\n" << endl;
    
    while (!glfwWindowShouldClose(window)) {
        // 计算时间差
        float currentFrame = (float)glfwGetTime();
        delta_time = currentFrame - last_frame;
        last_frame = currentFrame;
        
        // 处理输入
        processInput(window);
        
        // 物理更新
        for (auto& sphere : spheres) {
            sphere.update(delta_time);
        }
        
        // 碰撞检测和响应
        for (size_t i = 0; i < spheres.size(); ++i) {
            for (size_t j = i + 1; j < spheres.size(); ++j) {
                if (spheres[i].checkCollision(spheres[j])) {
                    spheres[i].resolveCollision(spheres[j]);
                }
            }
        }
        
        // 更新实例化数据
        for (size_t i = 0; i < spheres.size(); ++i) {
            instancePositions[i] = spheres[i].position;
            instanceVelocities[i] = spheres[i].velocity;
            instanceColors[i] = spheres[i].color;
            instanceRadii[i] = spheres[i].radius;
            instanceMasses[i] = spheres[i].mass;
        }
        
        // 更新缓冲区
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * MAX_SPHERES, instancePositions.data());
        
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * MAX_SPHERES, instanceVelocities.data());
        
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[2]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * MAX_SPHERES, instanceColors.data());
        
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[3]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * MAX_SPHERES, instanceRadii.data());
        
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[4]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * MAX_SPHERES, instanceMasses.data());
        
        // 渲染
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(shaderProgram);
        
        // 设置相机矩阵
        mat4 view = lookAt(camera_pos, camera_pos + camera_front, camera_up);
        mat4 projection = perspective(radians(camera_fov), 
                                    (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 
                                    0.1f, 100.0f);
        mat4 model = mat4(1.0f);
        
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, value_ptr(projection));
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, value_ptr(camera_pos));
        glUniform1f(glGetUniformLocation(shaderProgram, "time"), currentFrame);
        
        // 方向光设置
        glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.ambient"), 0.2f, 0.2f, 0.3f);
        glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.diffuse"), 0.8f, 0.8f, 0.7f);
        glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.specular"), 1.0f, 1.0f, 1.0f);
        
        // 点光源设置
        glUniform1i(glGetUniformLocation(shaderProgram, "numPointLights"), 2);
        
        // 第一个点光源（动态）
        vec3 lightPos1 = vec3(sin(currentFrame) * 8.0f, 5.0f, cos(currentFrame) * 8.0f);
        glUniform3fv(glGetUniformLocation(shaderProgram, "pointLights[0].position"), 1, value_ptr(lightPos1));
        glUniform3f(glGetUniformLocation(shaderProgram, "pointLights[0].ambient"), 0.1f, 0.1f, 0.2f);
        glUniform3f(glGetUniformLocation(shaderProgram, "pointLights[0].diffuse"), 1.0f, 0.3f, 0.3f);
        glUniform3f(glGetUniformLocation(shaderProgram, "pointLights[0].specular"), 1.0f, 0.3f, 0.3f);
        glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].linear"), 0.09f);
        glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[0].quadratic"), 0.032f);
        
        // 第二个点光源（动态）
        vec3 lightPos2 = vec3(cos(currentFrame * 1.5f) * 6.0f, 3.0f, sin(currentFrame * 1.5f) * 6.0f);
        glUniform3fv(glGetUniformLocation(shaderProgram, "pointLights[1].position"), 1, value_ptr(lightPos2));
        glUniform3f(glGetUniformLocation(shaderProgram, "pointLights[1].ambient"), 0.1f, 0.2f, 0.1f);
        glUniform3f(glGetUniformLocation(shaderProgram, "pointLights[1].diffuse"), 0.3f, 1.0f, 0.3f);
        glUniform3f(glGetUniformLocation(shaderProgram, "pointLights[1].specular"), 0.3f, 1.0f, 0.3f);
        glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].linear"), 0.09f);
        glUniform1f(glGetUniformLocation(shaderProgram, "pointLights[1].quadratic"), 0.032f);
        
        // 渲染球体
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)sphereIndices.size(), GL_UNSIGNED_INT, 0, MAX_SPHERES);
        
        // 交换缓冲区
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // 清理资源
    cout << "\nCleaning up resources..." << endl;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(5, instanceVBOs);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    
    cout << "Program exited successfully!" << endl;
    system("pause");
    return 0;
}