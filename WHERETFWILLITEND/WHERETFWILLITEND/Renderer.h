#pragma once
#include "RenderingSystem.h"
#include "Window.h"
#include "G_buffer.h"

struct alignas(256) CbPerPass {
    XMFLOAT4X4 model;
    XMFLOAT4X4 inv_model;
    XMFLOAT4X4 view;
    XMFLOAT4X4 inv_view;
    XMFLOAT4X4 projection;
    XMFLOAT4X4 inv_projection;
    XMFLOAT3 cameraPos;
    float pad0;
    float time;
    float nearZ;
    float farZ;
    float pad1;
};
struct alignas(256) CbLight {
    XMFLOAT3 lightPos;
    float pad0;
    XMFLOAT3 lightDir;  
    float lightType;    
    XMFLOAT4 ambient_k;
    XMFLOAT4 diffuse_k;
    XMFLOAT4 specular_k;
    float shiny_k;
    float intensity;
    float range;
    float spotCutoff; 
    XMFLOAT2 screenSize;
    float pad1[2];
};
struct alignas(256) CbMaterial {
    float time;
    float pad[3];
};

class Renderer{
private:
    RenderingSystem render_system_;
    ComPtr<ID3D12Device> device_;
    ComPtr<IDXGISwapChain3> swap_chain_;
    ComPtr<ID3D12CommandQueue> command_queue_;
    ComPtr<ID3D12CommandAllocator> command_allocator_;
    ComPtr<ID3D12GraphicsCommandList> command_list_;
    ComPtr<ID3D12Fence> fence_;
    UINT fence_value_ = 0;
    std::vector<UINT64> frame_fence_values_;
    ComPtr<ID3D12DescriptorHeap> rtv_heap_;
    UINT rtv_descriptor_size_;
    ComPtr<ID3D12DescriptorHeap> dsv_heap_;
    UINT dsv_descriptor_size_;
    ComPtr<ID3D12DescriptorHeap> cbv_srv_uav_heap_;
    UINT cbv_srv_uav_descriptor_size_;
    ComPtr<ID3D12DescriptorHeap> sampler_heap_;
    UINT sampler_descriptor_size_;
    UINT frame_count_; 
    std::vector<ComPtr<ID3D12Resource>> render_targets_;
    UINT width_;
    UINT height_;
    D3D12_VIEWPORT viewport_;
    D3D12_RECT scissor_rect_;
    UINT current_backbuffer_ = 0;
    ComPtr<ID3D12RootSignature> geom_root_signature_;
    ComPtr<ID3D12RootSignature> light_root_signature_;
    ComPtr<ID3D12PipelineState> geom_pipeline_state_;
    ComPtr<ID3D12PipelineState> light_pipeline_stat_;
    ComPtr<ID3D12Resource> vertex_buffer_;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;
    ComPtr<ID3D12Resource> cb_perpass_;
    ComPtr<ID3D12Resource> cb_light_;
    std::vector<ComPtr<ID3D12Resource>> textures_;
    std::vector<ComPtr<ID3D12Resource>> material_cb_;
    void* cb_perpass_mapped_;
    void* cb_light_mapped_;
    std::vector<void*> material_cb_mapped_;
    UINT sample_amount_=1;
    UINT msaa_quality_ = 0;
    UINT vertex_count_;
    TGAImage dummy_;
    GBuffer g_buffer_;
    //msaa bullsh**
    ComPtr<ID3D12Resource> msaa_render_target_;
    ComPtr<ID3D12DescriptorHeap> rtv_msaa_heap_;
    D3D12_CPU_DESCRIPTOR_HANDLE msaa_rtv_handle_;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_;
    HANDLE eventHandle_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    bool first_frame_ = true;

    // step2
    void CreateGraphicsDevice(UINT width, UINT height, int frame_count);
    // step3.1
    void CreateFence();
    // step3.2
    void AskDescryptorSizes();
    // step4
    //void check4XMSAA();
    // step5
    void CreateCommandStuff();
    // step6
    void CreateSwapChain(HWND hwnd);
    // step7
    void CreateHeaps(int textures_amount);
    // step8
    void CreateRTV();
    // step10
    void ViewportScissorSetup();
    // graphic pipeline bull****
    void CreateVertexBuffer(Model& mesh);
    void CreateInputLayout();
    void LoadTextureFromTGA(TGAImage& image, UINT textureSlot = 0);
    void EnableDebugLayer();
    void CreateRenderingSystem();//g buffer inits there too
    void CreateCBV_SRV_Sampler(XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, Model mesh, XMFLOAT3 light_pos);
public:
    void Initialize(UINT width, UINT height, int frame_count, HWND hwnd, Model& mesh, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, XMFLOAT3 light_pos);
    void RenderFrame(Model& mesh, float time, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up);
};