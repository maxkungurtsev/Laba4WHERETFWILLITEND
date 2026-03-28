#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Model.h"

XMFLOAT3 Model::summ(XMFLOAT3& a, XMFLOAT3& b) {
    XMVECTOR va = XMLoadFloat3(&a);
    XMVECTOR vb = XMLoadFloat3(&b);
    XMVECTOR vAdd = va + vb;
    XMFLOAT3 sum;
    XMStoreFloat3(&sum, vAdd);
    return sum;
}
XMFLOAT3 Model::diff(XMFLOAT3& a, XMFLOAT3& b) {
    XMVECTOR va = XMLoadFloat3(&a);
    XMVECTOR vb = XMLoadFloat3(&b);
    XMVECTOR vAdd = va - vb;
    XMFLOAT3 sum;
    XMStoreFloat3(&sum, vAdd);
    return sum;
}
XMFLOAT2 Model::summ(XMFLOAT2& a, XMFLOAT2& b) {
    XMVECTOR va = XMLoadFloat2(&a);
    XMVECTOR vb = XMLoadFloat2(&b);
    XMVECTOR vAdd = va + vb;
    XMFLOAT2 sum;
    XMStoreFloat2(&sum, vAdd);
    return sum;
}
XMFLOAT2 Model::diff(XMFLOAT2& a, XMFLOAT2& b) {
    XMVECTOR va = XMLoadFloat2(&a);
    XMVECTOR vb = XMLoadFloat2(&b);
    XMVECTOR vAdd = va - vb;
    XMFLOAT2 sum;
    XMStoreFloat2(&sum, vAdd);
    return sum;
}

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
        OutputDebugStringA(("material " + std::to_string(i)+'\n').c_str());
        for (int type = aiTextureType_NONE; type <= aiTextureType_UNKNOWN; type++)
        {
            aiTextureType texType = (aiTextureType)type;

            unsigned int count = mat->GetTextureCount(texType);
            OutputDebugStringA(("amount of "+std::to_string(texType) + " is "+ std::to_string(count) +'\n').c_str());
        }
        


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

        if (mat->GetTextureCount(aiTextureType_HEIGHT) > 0)
        {
            aiString path;
            mat->GetTexture(aiTextureType_HEIGHT, 0, &path);
            std::string path_ = path.C_Str();
            TGAImage image_tga;
            const Image* image_png;
            //choose between parsers
            if (path_.find("ddn") != std::string::npos) {
                outMat.hasNormTexture = true;
            }
            else if (path_.find("bump") != std::string::npos) {
                outMat.hasHeightTexture = true;
            }
            outMat.HeightNormTexPath = path_;
            OutputDebugStringA(("normal texture for material " + path_ + "loaded" + '\n').c_str());
            if (path_.substr(path_.size() - 3) == "tga") {
                image_tga.read_tga_file(path_.c_str());
                outMat.HeightNormTexture = std::make_shared<GTexture>(image_tga, path_, device, TextureUsage::Normalmap);
            }
            else {
                ScratchImage image;
                std::wstring wpath(path_.begin(), path_.end());
                HRESULT hr = LoadFromWICFile(wpath.c_str(), WIC_FLAGS_NONE, nullptr, image);
                if (FAILED(hr)) {
                    OutputDebugStringA(("normal texture for material " + outMat.HeightNormTexPath + " failed to load" + '\n').c_str());
               
                    throw std::runtime_error("failed loading normal texture from png");
                }
                image_png = image.GetImage(0, 0, 0);
                outMat.HeightNormTexture = std::make_shared<GTexture>(image_png, outMat.HeightNormTexPath, device, TextureUsage::Normalmap);
            }
        }
        else {
            // for sponza normal path is just not there
            //outMat.normalTexPath = "normal texture missing";
            std::string name = "no texture = no path";
            outMat.HeightNormTexture = std::make_shared<GTexture>(dummy_, name, device, TextureUsage::Normalmap);
            OutputDebugStringA(("normal texture for material " + std::to_string(i) + " is missing" + '\n').c_str());
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
            if (mesh->HasTangentsAndBitangents()){
                v.tangent = XMFLOAT3(mesh->mTangents[i][0],
                                     mesh->mTangents[i][1], 
                                     mesh->mTangents[i][2]);
                v.bitangent = XMFLOAT3(mesh->mBitangents[i][0],
                                       mesh->mBitangents[i][1],
                                       mesh->mBitangents[i][2]);
                //OutputDebugStringA((std::to_string(mesh->mBitangents[i][0])+" "+ std::to_string(mesh->mBitangents[i][1]) + " " + std::to_string(mesh->mBitangents[i][2]) + "\n").c_str());
            }
            vertices_.push_back(v);
        }
    std::vector<int> tangentCount(vertices_.size(), 0);
    std::vector<int> bitangentCount(vertices_.size(), 0);
        //meshs
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
    std::vector<int> tangentCount(vertices_.size(), 0);
    std::vector<int> bitangentCount(vertices_.size(), 0);
    for (unsigned m = 0; m < scene->mNumMeshes; ++m) {
        aiMesh* mesh = scene->mMeshes[m];
        SubMesh part=submeshes_[m];
        for (unsigned f = 0; f < mesh->mNumFaces; ++f){
            const aiFace& face = mesh->mFaces[f];
            // Triangulate is enabled, but keep guard anyway
            if (face.mNumIndices != 3) continue;
            // count tangents
            if (!mesh->HasTangentsAndBitangents()) {
                Vertex v1 = vertices_[part.baseVertex + face.mIndices[0]];
                Vertex v2 = vertices_[part.baseVertex + face.mIndices[1]];
                Vertex v3 = vertices_[part.baseVertex + face.mIndices[2]];
                //OutputDebugStringA(std::to_string(vertices_.size()).c_str());
                XMFLOAT3 edge1 = diff(v2.position, v1.position);
                XMFLOAT3 edge2 = diff(v3.position, v1.position);
                XMFLOAT2 deltaUV1 = diff(v2.uv, v1.uv);
                XMFLOAT2 deltaUV2 = diff(v3.uv, v1.uv);
                float f = 1 / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                XMFLOAT3 tangent;
                XMFLOAT3 bitangent;
                tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                vertices_[part.baseVertex + face.mIndices[0]].tangent = summ(vertices_[part.baseVertex + face.mIndices[0]].tangent, tangent);
                tangentCount[part.baseVertex + face.mIndices[0]] += 1;
                vertices_[part.baseVertex + face.mIndices[1]].tangent = summ(vertices_[part.baseVertex + face.mIndices[1]].tangent, tangent);
                tangentCount[part.baseVertex + face.mIndices[1]] += 1;
                vertices_[part.baseVertex + face.mIndices[2]].tangent = summ(vertices_[part.baseVertex + face.mIndices[2]].tangent, tangent);
                tangentCount[part.baseVertex + face.mIndices[2]] += 1;
                vertices_[part.baseVertex + face.mIndices[0]].bitangent = summ(vertices_[part.baseVertex + face.mIndices[0]].bitangent, bitangent);
                bitangentCount[part.baseVertex + face.mIndices[0]] += 1;
                vertices_[part.baseVertex + face.mIndices[1]].bitangent = summ(vertices_[part.baseVertex + face.mIndices[1]].bitangent, bitangent);
                bitangentCount[part.baseVertex + face.mIndices[1]] += 1;
                vertices_[part.baseVertex + face.mIndices[2]].bitangent = summ(vertices_[face.mIndices[2]].bitangent, bitangent);
                bitangentCount[part.baseVertex + face.mIndices[2]] += 1;
            }
        }
    }
    //усреднить тангенты
    for (int i = 0; i < vertices_.size(); i++) {
        vertices_[i].tangent = XMFLOAT3(vertices_[i].tangent.x / tangentCount[i], vertices_[i].tangent.y / tangentCount[i], vertices_[i].tangent.z / tangentCount[i]);
        vertices_[i].bitangent = XMFLOAT3(vertices_[i].bitangent.x / bitangentCount[i], vertices_[i].bitangent.y / bitangentCount[i], vertices_[i].bitangent.z / bitangentCount[i]);
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