#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include "tgaimage.h"

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
    TGAImage diffuseTexture;
    std::string normalTexPath;
    bool hasNormalTexture = false;
    TGAImage NormalTexture;
};
struct SubMesh
{
    size_t startVertex;
    size_t vertexCount;
    size_t materialIndex;
};
class Model
{
public:
    Model(const std::string& model_filename);
    const std::vector<Vertex>& GetVertices() const {return vertices_;}
    std::vector<MaterialData>& GetMaterials() {return materials_;}
    const std::vector<SubMesh>& GetSubMeshes() const {return submeshes_;}
private:
    std::vector<XMFLOAT3> positions_;
    std::vector<XMFLOAT2> texcoords_;
    std::vector<XMFLOAT3> normals_;
    std::vector<Vertex> vertices_;
    std::vector<MaterialData> materials_;
    std::vector<SubMesh> submeshes_;
    void buildVertices();
};