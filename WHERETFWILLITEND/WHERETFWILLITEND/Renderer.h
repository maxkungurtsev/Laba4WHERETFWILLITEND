#pragma once
#include <iostream>
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <vector>
#include "Window.h"
#include "Model.h"
#include <d3dcompiler.h>
using Microsoft::WRL::ComPtr;

struct alignas(256) MVPConstants{
    XMFLOAT4X4 model;
    XMFLOAT4X4 view;
    XMFLOAT4X4 projection;
};
struct alignas(256) LightConstants{
    XMFLOAT3 lightPos;
    float pad1;
    XMFLOAT3 cameraPos;
    float pad2;
};

class Renderer
{
private:
    ComPtr<ID3D12Device> device_;
    ComPtr<IDXGISwapChain3> swap_chain_;
    ComPtr<ID3D12CommandQueue> command_queue_;
    ComPtr<ID3D12CommandAllocator> command_allocator_;
    ComPtr<ID3D12GraphicsCommandList> command_list_;
    ComPtr<ID3D12Fence> fence;
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
    ComPtr<ID3DBlob> pixel_shader_;
    ComPtr<ID3D12Resource> mvp_cb_;
    ComPtr<ID3D12Resource> light_cb_;
    MVPConstants   mvp_data_;
    LightConstants light_data_;
    uint8_t* mvp_cb_mapped_ = nullptr;
    uint8_t* light_cb_mapped_ = nullptr;
    ComPtr<ID3D12Resource> texture_;

    // step2
    void CreateGraphicsDevice(UINT width, UINT height, int frame_count);
    // step3.1
    void CreateFence();
    // step3.2
    void AskDescryptorSizes();
    // step4
    bool check4XMSAA();
    // step5
    void CreateCommandStuff();
    // step6
    void CreateSwapChain(HWND hwnd);
    // step7
    void CreateHeaps();
    // step8
    void CreateRTV();
    // step9
     void CreateZBuffer();
    // step10
    void ViewportScissorSetup();
    // graphic pipeline bull****
    void CreateRootSignature();
    void CreatePipelineStateObject();
    void CreateVertexBuffer(const Model& mesh);
    void CompileShaders();
    void CreateConstantBuffers();
    void CreateInputLayout();
    void CreateSRVandSampler();
    void LoadTextureFromTGA(TGAImage& image, UINT textureSlot = 0);
public:
    void Initialize(UINT width, UINT height, int frame_count, HWND hwnd, const Model& mesh);
    void RenderFrame();
    void ViewportScissorSetup();
};