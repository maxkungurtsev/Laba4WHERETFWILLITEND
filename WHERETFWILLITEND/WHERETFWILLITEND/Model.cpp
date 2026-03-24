#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Model.h"
Model::Model(const std::string& filename, std::shared_ptr<Gdevice> device)
{
    Assimp::Importer importer;
    dummy_.read_tga_file("dummy.tga");
    const aiScene* scene = importer.ReadFile(
        filename,
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_FlipUVs);
    if (!scene || !scene->HasMeshes()){
        return;
    }
    vertices_.clear();
    materials_.clear();
    submeshes_.clear();
    //mats and diffuse textures
    materials_.resize(scene->mNumMaterials);
    for (unsigned i = 0; i < scene->mNumMaterials; ++i){
        aiMaterial* mat = scene->mMaterials[i];
        MaterialData& outMat = materials_[i];
        aiColor3D color;
        // ambient
        if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_AMBIENT, color))
            outMat.ambient_k = { color.r, color.g, color.b };
        // diffuse
        if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_DIFFUSE, color))
            outMat.diffuse_k = { color.r, color.g, color.b };
        // specular
        if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_SPECULAR, color))
            outMat.specular_k = { color.r, color.g, color.b };
        // shiny
        float shininess = 0;
        if (AI_SUCCESS == mat->Get(AI_MATKEY_SHININESS, shininess))
            outMat.shiny_k = shininess;
        // diffuse texture
        if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString path;
            mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
            outMat.diffuseTexPath = path.C_Str();
            outMat.hasDiffuseTexture = true;
            TGAImage image;
            image.read_tga_file(outMat.diffuseTexPath.c_str());

            OutputDebugStringA(("diffuse texture for material " + std::to_string(i) + " exists"+'\n').c_str());
            outMat.diffuseTexture = std::make_shared<GTexture>(image, outMat.diffuseTexPath, device, TextureUsage::Albedo);
        }
        else {
            outMat.diffuseTexPath = "diffuse texture missing";
            outMat.diffuseTexture = std::make_shared<GTexture>(dummy_, outMat.diffuseTexPath, device, TextureUsage::Albedo);
            OutputDebugStringA(("diffuse texture for material " + std::to_string(i) + " is missing" + '\n').c_str());
        }
    }
    // normal textures
    for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
        aiMaterial* mat = scene->mMaterials[i];
        MaterialData& outMat = materials_[i];
        if (mat->GetTextureCount(aiTextureType_NORMALS) > 0)
        {
            aiString path;
            mat->GetTexture(aiTextureType_NORMALS, 0, &path);
            outMat.normalTexPath = path.C_Str();
            outMat.hasNormalTexture = true;
            TGAImage image;
            image.read_tga_file(outMat.normalTexPath.c_str());
            OutputDebugStringA(("Normal texture for material " + std::to_string(i) + " exists"+'\n').c_str());
            outMat.NormalTexture = std::make_shared<GTexture>(image, outMat.normalTexPath, device, TextureUsage::Normalmap);
        }
        else {
            // for sponza normal path is just not there
            outMat.normalTexPath = "normal texture missing";
            outMat.NormalTexture = std::make_shared<GTexture>(dummy_, outMat.normalTexPath, device, TextureUsage::Normalmap);
            OutputDebugStringA(("normal texture for material " + std::to_string(i) + " is missing" + '\n').c_str());
           // OutputDebugStringA((outMat.normalTexPath + '\n').c_str());
        }
    }
    // meshs
    for (unsigned m = 0; m < scene->mNumMeshes; ++m){
        aiMesh* mesh = scene->mMeshes[m];
        SubMesh part;
        part.startVertex = vertices_.size();
        part.materialIndex = mesh->mMaterialIndex;
        //verts
        for (unsigned i = 0; i < mesh->mNumVertices; ++i){
            Vertex v{};
            v.position = {
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            };
            if (mesh->HasNormals())
            {
                v.normal = {
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z
                };
            }
            if (mesh->mTextureCoords[0])
            {
                v.uv = {
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                };
            }
            vertices_.push_back(v);
        }
        part.vertexCount = vertices_.size() - part.startVertex;
        submeshes_.push_back(part);
    }
}
void Model::buildVertices()
{
    size_t vertexCount = positions_.size();
    if (normals_.size() < positions_.size())
        normals_.resize(positions_.size(), XMFLOAT3{ 0.0f, 0.0f, 0.0f });
    if (texcoords_.size() < positions_.size())
        texcoords_.resize(positions_.size(), XMFLOAT2{ 0.0f, 0.0f });
    vertices_.reserve(vertexCount);
    for (size_t i = 0; i < vertexCount; ++i){
        Vertex v;
        v.position = positions_[i];
        v.normal = normals_[i];
        v.uv = texcoords_[i];
        vertices_.push_back(v);
    }
}