#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
using namespace std;
const char *vertexShaderSource = R"(
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal=mat3(transpose(inverse(model)))*aNormal;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char *fragmentShaderSource = R"(
#version 330 core 
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos;

void main()
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
)";

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
};

class Mesh
{
public:
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    unsigned int vao;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices)
    {
        this->indices = indices;
        this->vertices = vertices;

        this->setupMesh();
    }

    void Draw(unsigned int shaderProgram)
    {
        glBindVertexArray(this->vao);
        glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
private:
    unsigned int vbo, ebo;
    void setupMesh()
    {
        glGenVertexArrays(1,&this->vao);
        glGenBuffers(1,&this->vbo);
        glGenBuffers(1,&this->ebo);

        glBindVertexArray(this->vao);
        glBindBuffer(GL_ARRAY_BUFFER,this->vbo);
        glBufferData(GL_ARRAY_BUFFER,this->vertices.size()*sizeof(Vertex),&this->vertices[0],GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,this->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,this->indices.size()*sizeof(unsigned int),&this->indices[0],GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,Normal));
        glBindVertexArray(0);
    }
};

class Model
{
public:
    vector<Mesh> meshes;
    string directory;
    Model(const char *path)
    {
        this->loadModel(path);
    }
    void Draw(unsigned int shaderProgram)
    {
        for(unsigned int i=0;i<this->meshes.size();i++)
        {
            this->meshes[i].Draw(shaderProgram);
        }
    }
private:
    void loadModel(string path)
    {
        Assimp::Importer importer;
        const aiScene *scene=importer.ReadFile(path,aiProcess_Triangulate | aiProcess_FlipUVs);
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            cout<<"ERROR::ASSIMP::"<<importer.GetErrorString()<<endl;
            return;
        }
        this->directory=path.substr(0,path.find_last_of('/'));
        this->processNode(scene->mRootNode,scene);
    }
    //处理节点对应的所哟网络
    void processNode(aiNode *node,const aiScene *scene)
    {
        for(unsigned int i=0;i<node->mNumMeshes;i++)
        {
            aiMesh *mesh=scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(this->processMesh(mesh,scene));
        }
        for(unsigned int i=0;i<node->mNumChildren;i++)
        {
            this->processNode(node->mChildren[i],scene);
        }
    }
    Mesh processMesh(aiMesh *mesh,const aiScene *scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        for(unsigned int i=0;i<mesh->mNumVertices;i++)
        {
            Vertex vertex;
            vertex.Position=glm::vec3(mesh->mVertices[i].x,mesh->mVertices[i].y,mesh->mVertices[i].z);
            if(mesh->HasNormals())
            {
                vertex.Normal=glm::vec3(mesh->mNormals[i].x,mesh->mNormals[i].y,mesh->mNormals[i].z);
            }
            else
            {
                vertex.Normal = glm::normalize(glm::cross(
                glm::vec3(mesh->mVertices[i+1].x-mesh->mVertices[i].x,
                mesh->mVertices[i+1].y-mesh->mVertices[i].y,
                mesh->mVertices[i+1].z-mesh->mVertices[i].z),
                glm::vec3(mesh->mVertices[i+2].x-mesh->mVertices[i].x,
                  mesh->mVertices[i+2].y-mesh->mVertices[i].y,
                  mesh->mVertices[i+2].z-mesh->mVertices[i].z)
    ));
            }
            
            vertices.push_back(vertex);
        }
        for(unsigned int i=0;i<mesh->mNumFaces;i++)
        {
            aiFace face=mesh->mFaces[i];
            for(unsigned int j=0;j<face.mNumIndices;j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }
        return Mesh(vertices,indices);
    }
};
unsigned int  createShader()
{
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cerr << "Vertex shader compilation failed:\n" << infoLog << endl;
    }
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cerr << "Fragment shader compilation failed:\n" << infoLog << endl;
    }
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cerr << "Shader program linking failed:\n" << infoLog << endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if(glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize GLEW" << endl;
        glfwTerminate();
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    unsigned int shaderProgram=createShader();
    Model ourmodel("D:/OpenGL/program/after_eight_month/_09_04/cube.obj");

    while(!glfwWindowShouldClose(window))
    {
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        
        unsigned int mdoelloc=glGetUniformLocation(shaderProgram, "model");
        unsigned int viewloc=glGetUniformLocation(shaderProgram, "view");
        unsigned int projloc=glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(mdoelloc,1,GL_FALSE,glm::value_ptr(model));
        glUniformMatrix4fv(viewloc,1,GL_FALSE,glm::value_ptr(view));
        glUniformMatrix4fv(projloc,1,GL_FALSE,glm::value_ptr(projection));

        //设置光照
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.5f, 0.31f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 0.0f, 0.0f, 5.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), 0.0f, 0.0f, 0.0f);
        
        ourmodel.Draw(shaderProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}