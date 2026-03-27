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
    if (!scene || !scene->HasMeshes()) {
        return;
    }
    vertices_.clear();
    materials_.clear();
    submeshes_.clear();
    //mats and diffuse textures
    materials_.resize(scene->mNumMaterials);
    OutputDebugStringA("material amount:\n");
    OutputDebugStringA((std::to_string(materials_.size()) + '\n').c_str());
    for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
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
            TGAImage image_tga;
            const Image* image_png;
            //choose between parsers
            if (outMat.diffuseTexPath.substr(outMat.diffuseTexPath.size() - 3) == "tga"){
            image_tga.read_tga_file(outMat.diffuseTexPath.c_str());
            OutputDebugStringA(("diffuse texture for material " + std::to_string(i) + " exists"+'\n').c_str());
            outMat.diffuseTexture = std::make_shared<GTexture>(image_tga, outMat.diffuseTexPath, device, TextureUsage::Albedo);
            }
            else {
                ScratchImage image;
                std::wstring wpath(outMat.diffuseTexPath.begin(), outMat.diffuseTexPath.end());
                HRESULT hr = LoadFromWICFile(wpath.c_str(), WIC_FLAGS_NONE, nullptr, image);
                if (FAILED(hr)) {
                    throw std::runtime_error("failed loading texture from png");
                }
                image_png = image.GetImage(0, 0, 0);
                outMat.diffuseTexture = std::make_shared<GTexture>(image_png, outMat.diffuseTexPath, device, TextureUsage::Albedo);
            }


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
        OutputDebugStringA((std::to_string(mat->GetTextureCount(aiTextureType_HEIGHT))).c_str());

        if (mat->GetTextureCount(aiTextureType_HEIGHT) > 0)
        {
            aiString path;
            mat->GetTexture(aiTextureType_HEIGHT, 0, &path);
            outMat.HeightTexPath = path.C_Str();
            outMat.hasHeightTexture = true;
            TGAImage image_tga;
            const Image* image_png;
            //choose between parsers
            OutputDebugStringA(("normal texture for material " + outMat.HeightTexPath + "loaded" + '\n').c_str());
            if (outMat.HeightTexPath.substr(outMat.HeightTexPath.size() - 3) == "tga") {
                image_tga.read_tga_file(outMat.HeightTexPath.c_str());
                outMat.HeightTexture = std::make_shared<GTexture>(image_tga, outMat.HeightTexPath, device, TextureUsage::Normalmap);
            }
            else {
                ScratchImage image;
                std::wstring wpath(outMat.HeightTexPath.begin(), outMat.HeightTexPath.end());
                HRESULT hr = LoadFromWICFile(wpath.c_str(), WIC_FLAGS_NONE, nullptr, image);
                if (FAILED(hr)) {
                    OutputDebugStringA(("normal texture for material " + outMat.HeightTexPath + " failed to load" + '\n').c_str());
               
                    throw std::runtime_error("failed loading normal texture from png");
                }
                image_png = image.GetImage(0, 0, 0);
                outMat.HeightTexture = std::make_shared<GTexture>(image_png, outMat.HeightTexPath, device, TextureUsage::Normalmap);
            }
        }
        else {
            // for sponza normal path is just not there
            //outMat.normalTexPath = "normal texture missing";
            outMat.HeightTexture = std::make_shared<GTexture>(dummy_, outMat.HeightTexPath, device, TextureUsage::Normalmap);
            OutputDebugStringA(("normal texture for material " + outMat.HeightTexPath + " is missing" + '\n').c_str());
           // OutputDebugStringA((outMat.normalTexPath + '\n').c_str());
        }
    }
    // meshs
    for (unsigned m = 0; m < scene->mNumMeshes; ++m){
        aiMesh* mesh = scene->mMeshes[m];
        SubMesh part;
        part.baseVertex = vertices_.size();
        part.firstIndex = indices.size();
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
        for (unsigned f = 0; f < mesh->mNumFaces; ++f)
        {
            const aiFace& face = mesh->mFaces[f];

            // Triangulate is enabled, but keep guard anyway
            if (face.mNumIndices != 3) continue;

            indices.push_back(part.baseVertex + face.mIndices[0]);
            indices.push_back(part.baseVertex + face.mIndices[1]);
            indices.push_back(part.baseVertex + face.mIndices[2]);
        }

        part.indexCount = static_cast<uint32_t>(indices.size()) - part.firstIndex;
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