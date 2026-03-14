#pragma once
#include <d3d12.h> 
#include <dxgi1_6.h>
#include <windows.h>
#include <wrl.h>  
#include <d3dcompiler.h>
#include <iostream>
#include <vector>
#include <sstream>
#include "Window.h"
#include "Model.h"
using Microsoft::WRL::ComPtr;

struct alignas(256) MVPConstants {
    XMFLOAT4X4 model;
    XMFLOAT4X4 view;
    XMFLOAT4X4 projection;
    float time;
    float pad[3];
};
struct alignas(256) LightConstants {
    XMFLOAT3 lightPos;
    float pad1;
    XMFLOAT3 cameraPos;
    float pad2;
    XMFLOAT4 ambient_k = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f);
    XMFLOAT4 diffuse_k = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f);
    XMFLOAT4 specular_k = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f);
    float shiny_k = 0.8f;
    float intensity = 5.0f;
    float pad4 = 0.0;
    float time;
    float pad3[3] = { 0.0,0.0,0.0 };
};

class RenderingSystem
{
private:
    ComPtr<ID3D12Device> device_;
    ComPtr<IDXGISwapChain3> swap_chain_;
    ComPtr<ID3D12CommandQueue> command_queue_;
    ComPtr<ID3D12CommandAllocator> command_allocator_;
    ComPtr<ID3D12GraphicsCommandList> command_list_;
    ComPtr<ID3D12Fence> fence_;
    UINT fence_value_ = 0;
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
    ComPtr<ID3D12Resource> z_buffer_;
    UINT width_;
    UINT height_;
    D3D12_VIEWPORT viewport_;
    D3D12_RECT scissor_rect_;
    UINT current_backbuffer_ = 0;
    ComPtr<ID3D12RootSignature> root_signature_;
    ComPtr<ID3D12PipelineState> pipeline_state_;
    std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout_;
    ComPtr<ID3D12Resource> vertex_buffer_;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;
    ComPtr<ID3DBlob> vertex_shader_;
    ComPtr<ID3DBlob> vertex_shader_anim_;
    ComPtr<ID3DBlob> pixel_shader_;
    ComPtr<ID3D12Resource> mvp_cb_;
    ComPtr<ID3D12Resource> light_cb_;
    MVPConstants   mvp_data_;
    LightConstants light_data_;
    std::vector<ComPtr<ID3D12Resource>> textures_;
    UINT sample_amount_ = 1;
    UINT vertex_count_;
    void* mvp_cb_mapped_;
    void* light_cb_mapped_;
    TGAImage dummy_;
    //msaa bullsh**
    ComPtr<ID3D12Resource> msaa_render_target_;
    ComPtr<ID3D12DescriptorHeap> rtv_msaa_heap_;
    D3D12_CPU_DESCRIPTOR_HANDLE msaa_rtv_handle_;
    bool first_frame_ = true;
    void AskDescryptorSizes();
    // step6
    void CreateHeaps(int textures_amount);
    // step8
    void CreateRTV();
    // step9
    void CreateZBuffer();
    // step10
    void ViewportScissorSetup();
    // graphic pipeline bull****
    void CreateRootSignature(int textures_amount);
    void CreatePipelineStateObject();
    void CreateVertexBuffer(Model& mesh);
    void CompileShaders();
    void CreateCBV_SRV_Sampler(XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, Model mesh, XMFLOAT3 light_pos);
    void CreateInputLayout();
    void LoadTextureFromTGA(TGAImage& image, UINT textureSlot = 0);
    void EnableDebugLayer();
public:
    void Initialize(UINT width, UINT height, int frame_count, Model& mesh, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, XMFLOAT3 light_pos,
        ComPtr<ID3D12Device> device,
        ComPtr<IDXGISwapChain3> swap_chain,
        ComPtr<ID3D12CommandQueue> command_queue,
        ComPtr<ID3D12CommandAllocator> command_allocator,
        ComPtr<ID3D12Fence> fence,
        UINT fence_value);
    void RenderFrame(Model& mesh, float time, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up);
};