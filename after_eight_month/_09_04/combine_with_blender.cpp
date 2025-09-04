#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

int main() {
    Assimp::Importer importer;
    std::cout << "Assimp successfully compiled and linked!" << std::endl;
    
    // 使用实际的模型文件路径
    const aiScene* scene = importer.ReadFile("D:/OpenGL/program/after_eight_month/_09_04/cube.obj", 
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
    
    if (!scene) {
        std::cout << "Error loading model: " << importer.GetErrorString() << std::endl;
        system("pause");
        return -1;
    }
    
    // 验证模型加载结果
    std::cout << "Model loaded successfully!" << std::endl;
    std::cout << "Number of meshes: " << scene->mNumMeshes << std::endl;
    std::cout << "Number of materials: " << scene->mNumMaterials << std::endl;
    std::cout << "Number of animations: " << scene->mNumAnimations << std::endl;
    std::cout << "Number of cameras: " << scene->mNumCameras << std::endl;
    std::cout << "Number of lights: " << scene->mNumLights << std::endl;
    
    // 打印第一个网格的信息
    if (scene->mNumMeshes > 0) {
        const aiMesh* mesh = scene->mMeshes[0];
        std::cout << "First mesh information:" << std::endl;
        std::cout << "  Number of vertices: " << mesh->mNumVertices << std::endl;
        std::cout << "  Number of faces: " << mesh->mNumFaces << std::endl;
        std::cout << "  Has normals: " << (mesh->HasNormals() ? "Yes" : "No") << std::endl;
        std::cout << "  Has UVs: " << (mesh->HasTextureCoords(0) ? "Yes" : "No") << std::endl;
    }
    
    system("pause");
    return 0;
}