#pragma once
#include "Graphics\GTexture.h"
#include "Graphics\Constants.h"
#include "Graphics\CBuffer.h"
#include "OctreeNode.h"
struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT2 uv;
    XMFLOAT3 tangent;
    XMFLOAT3 bitangent;

};
struct MaterialData
{
    XMFLOAT3 ambient_k = { 0,0,0 };
    XMFLOAT3 diffuse_k = { 1,1,1 };
    XMFLOAT3 specular_k = { 0,0,0 };
    float shiny_k = 32.0f;
    //diffuse
    std::string diffuseTexPath;
    bool hasDiffuseTexture = false;
    std::shared_ptr<GTexture> diffuseTexture;
    //normal
    std::string HeightNormTexPath;
    bool hasHeightTexture = false;
    bool hasNormTexture = false;
    std::shared_ptr<GTexture> HeightNormTexture;
    //roughness
    std::string roughnessTexPath;
    bool hasRoughnessTexture = false;
    std::shared_ptr<GTexture> roughnessTexture;

    //metallic
    std::string metallicTexPath;
    bool hasMetallicTexture = false;
    std::shared_ptr<GTexture> metallicTexture;

    //ambent occolision
    std::string AOTexPath;
    bool hasAOTexture = false;
    std::shared_ptr<GTexture> AOTexture;
};
struct SubMesh
{
    BoundingSphere bounding_sphere_;
    uint32_t firstIndex;
    uint32_t indexCount;
    uint32_t materialIndex;
    uint32_t baseVertex;
};
class Model
{
public:
    Model(const std::string& model_filename, std::shared_ptr<Gdevice> device, bool billboardable, bool is_billboard, XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale);
    const std::vector<uint32_t>& Getindices() const { return indices; }
    const std::vector<Vertex>& GetVertices() const {return vertices_;}
    std::vector<MaterialData>& GetMaterials() {return materials_;}
    const std::vector<SubMesh>& GetSubMeshes() const {return submeshes_;}
    BoundingBox& GetBoundBox() { return mesh_box_; }
    XMFLOAT4& GetPosition() { return position_; }
    XMFLOAT3& GetRotation() { return rotation_; }
    XMFLOAT3& GetScale() { return scale_; }
    bool GetBillBoardable() { return billboardable_; }
    D3D12_VERTEX_BUFFER_VIEW GetVBV() { return vertex_buffer_view_; }
    D3D12_INDEX_BUFFER_VIEW GetIBV() { return index_buffer_view_; }
    void MakeOctree();
    void check() {
        if (octree_) {
            OutputDebugStringA("octree present\n");
        }
        else {
            throw std::runtime_error("octree not prese");
        }
    }
    void SetBilboard(std::shared_ptr<Model> bilboard) { bilboard_ = bilboard; }
    std::shared_ptr<Model> GetBilboard() { return bilboard_; }
    std::shared_ptr<OctreeNode> GetOctree() { return octree_; }
    bool IsBilboard() {return is_bilboard_; }
    const std::string& GetName() { return name; }
    std::shared_ptr<Cbuffer<MaterialConstants>> GetMaterialBuffer() { return mat_buffer_; }
private:
    std::shared_ptr <Cbuffer<MaterialConstants>> mat_buffer_;
    std::shared_ptr<Gdevice> device_;
    bool billboardable_ = false;
    bool is_bilboard_ = false;
    XMFLOAT4 position_ = { 0,0,0,1 };
    XMFLOAT3 rotation_ = { 0,0,0 };
    XMFLOAT3 scale_ = {1,1,1};
    TGAImage dummy_;
    const Image* Whitedummy_;
    UINT vertex_count_;
    std::string name;
    ComPtr<ID3D12Resource> vertex_buffer_;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;
    ComPtr<ID3D12Resource> index_buffer_;
    ComPtr<ID3D12Resource> index_buffer_upload_buffer_;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view_;
    std::vector<Vertex> vertices_;
    std::vector<MaterialData> materials_;
    std::vector<uint32_t> indices;
    std::vector<SubMesh> submeshes_;
    BoundingBox mesh_box_;
    std::shared_ptr<OctreeNode> octree_;
    std::shared_ptr<Model> bilboard_;
    XMFLOAT3 summ(XMFLOAT3& a, XMFLOAT3& b);
    XMFLOAT3 diff(XMFLOAT3& a, XMFLOAT3& b);
    XMFLOAT2 summ(XMFLOAT2& a, XMFLOAT2& b);
    XMFLOAT2 diff(XMFLOAT2& a, XMFLOAT2& b);
    void CreateIndexBuffer();
    void CreateVertexBuffer();
};