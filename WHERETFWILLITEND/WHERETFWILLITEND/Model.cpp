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

void Model::CreateVertexBuffer() {
    vertex_count_ = vertices_.size();
    UINT bufferSize = vertex_count_ * sizeof(Vertex);
    D3D12_RESOURCE_DESC bufferDesc{};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = bufferSize;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    D3D12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    HRESULT hr = device_->GetDXDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertex_buffer_));
    if (FAILED(hr))
        throw std::runtime_error("Failed to create vertex buffer");
    void* mappedData = nullptr;
    D3D12_RANGE readRange{ 0, 0 };
    hr = vertex_buffer_->Map(0, &readRange, &mappedData);
    if (FAILED(hr))
        throw std::runtime_error("Failed to fill vertex buffer");
    memcpy(mappedData, vertices_.data(), bufferSize);
    vertex_buffer_->Unmap(0, nullptr);
    vertex_buffer_view_.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
    vertex_buffer_view_.StrideInBytes = sizeof(Vertex);
    vertex_buffer_view_.SizeInBytes = bufferSize;
}

void Model::CreateIndexBuffer() {
    //device_->cmd_->ResetAllocator();

    UINT32 bufferSize = static_cast<UINT32>(indices.size() * sizeof(uint32_t));
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    device_->GetDXDevice()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&index_buffer_));

    void* mapped = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    index_buffer_->Map(0, &readRange, &mapped);
    memcpy(mapped, indices.data(), static_cast<size_t>(bufferSize));
    index_buffer_->Unmap(0, nullptr);
    index_buffer_view_.BufferLocation = index_buffer_->GetGPUVirtualAddress();
    index_buffer_view_.SizeInBytes = bufferSize;
    index_buffer_view_.Format = DXGI_FORMAT_R32_UINT;
}

Model::Model(const std::string& filename, std::shared_ptr<Gdevice> device, bool billboardable, bool is_billboard, XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale)
{
    name = filename;
    position_ = XMFLOAT4(pos.x, pos.y, pos.z, 1);
    rotation_ = rot;
    scale_ = scale;
    device_ = device;
    billboardable_ = billboardable;
    is_bilboard_ = is_billboard;
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
    XMFLOAT3 minPt = { FLT_MAX, FLT_MAX, FLT_MAX };
    XMFLOAT3 maxPt = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
    for (unsigned m = 0; m < scene->mNumMeshes; ++m) {
        aiMesh* mesh = scene->mMeshes[m];
        
        SubMesh part=submeshes_[m]; 
        std::vector<DirectX::XMFLOAT3> points;
        points.reserve(part.indexCount);

        for (uint32_t i = 0; i < part.indexCount; ++i)
        {
            uint32_t index = indices[part.firstIndex + i];
            uint32_t vertexIndex =index;
            points.push_back(vertices_[vertexIndex].position);
        }
        
        BoundingSphere::CreateFromPoints(submeshes_[m].bounding_sphere_,points.size(),points.data(),sizeof(DirectX::XMFLOAT3));
        XMFLOAT3 c = submeshes_[m].bounding_sphere_.Center;
        float r = submeshes_[m].bounding_sphere_.Radius;

        XMFLOAT3 smin = { c.x - r, c.y - r, c.z - r };
        XMFLOAT3 smax = { c.x + r, c.y + r, c.z + r };

        minPt.x = min(minPt.x, smin.x);
        minPt.y = min(minPt.y, smin.y);
        minPt.z = min(minPt.z, smin.z);

        maxPt.x = max(maxPt.x, smax.x);
        maxPt.y = max(maxPt.y, smax.y);
        maxPt.z = max(maxPt.z, smax.z);

        for (unsigned f = 0; f < mesh->mNumFaces; ++f){
            const aiFace& face = mesh->mFaces[f];

            if (face.mNumIndices != 3) continue;
            // count tangents
            if (!mesh->HasTangentsAndBitangents()) {
                Vertex v1 = vertices_[part.baseVertex + face.mIndices[0]];
                Vertex v2 = vertices_[part.baseVertex + face.mIndices[1]];
                Vertex v3 = vertices_[part.baseVertex + face.mIndices[2]];
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
                vertices_[part.baseVertex + face.mIndices[2]].bitangent = summ(vertices_[part.baseVertex + face.mIndices[2]].bitangent, bitangent);
                bitangentCount[part.baseVertex + face.mIndices[2]] += 1;
            }
        }
    }
    XMVECTOR minVec = XMLoadFloat3(&minPt);
    XMVECTOR maxVec = XMLoadFloat3(&maxPt);
    BoundingBox::CreateFromPoints(mesh_box_, minVec, maxVec);
    //усреднить тангенты
    for (int i = 0; i < vertices_.size(); i++) {
        if (tangentCount[i] > 0) {
            vertices_[i].tangent = XMFLOAT3(vertices_[i].tangent.x / tangentCount[i], vertices_[i].tangent.y / tangentCount[i], vertices_[i].tangent.z / tangentCount[i]);
        }
        if (bitangentCount[i] > 0) {
            vertices_[i].bitangent = XMFLOAT3(vertices_[i].bitangent.x / bitangentCount[i], vertices_[i].bitangent.y / bitangentCount[i], vertices_[i].bitangent.z / bitangentCount[i]);
        }
    }
    CreateVertexBuffer();
    CreateIndexBuffer();
    MakeOctree();
}


void Model::MakeOctree() {
    std::vector<int> all_submesh_indices;
    std::vector<BoundingSphere> all_spheres;
    for (int i = 0; i < submeshes_.size(); i++) {
        all_submesh_indices.push_back(i);
        all_spheres.push_back(submeshes_[i].bounding_sphere_);
    }
    octree_ = std::make_shared<OctreeNode>(0, mesh_box_, true, all_submesh_indices, all_spheres);
}