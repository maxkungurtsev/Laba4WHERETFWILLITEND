#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include "Graphics\GTexture.h"
#include <DirectXTex.h>
using namespace DirectX;
struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT2 uv;
};
struct MaterialData
{
    XMFLOAT3 ambient_k = { 0,0,0 };
    XMFLOAT3 diffuse_k = { 1,1,1 };
    XMFLOAT3 specular_k = { 0,0,0 };
    float shiny_k = 32.0f;
    std::string diffuseTexPath;
    bool hasDiffuseTexture = false;
    std::shared_ptr<GTexture> diffuseTexture;
    std::string HeightTexPath;
    bool hasHeightTexture = false;
    std::shared_ptr<GTexture> HeightTexture;
};
struct SubMesh
{
    size_t firstIndex;
    size_t indexCount;
    size_t materialIndex;
    size_t baseVertex;
};
class Model
{
public:
    Model(const std::string& model_filename, std::shared_ptr<Gdevice> device);
    const std::vector<size_t>& Geindices() const { return indices; }
    const std::vector<Vertex>& GetVertices() const {return vertices_;}
    std::vector<MaterialData>& GetMaterials() {return materials_;}
    const std::vector<SubMesh>& GetSubMeshes() const {return submeshes_;}
private:
    TGAImage dummy_;
    std::vector<XMFLOAT3> positions_;
    std::vector<XMFLOAT2> texcoords_;
    std::vector<XMFLOAT3> normals_;
    std::vector<Vertex> vertices_;
    std::vector<MaterialData> materials_;
    std::vector<size_t> indices;
    std::vector<SubMesh> submeshes_;
    void buildVertices();
};