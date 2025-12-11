#pragma once
#include <iostream>
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <vector>
#include "Window.h"
using Microsoft::WRL::ComPtr;

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
public:
    void Initialize(UINT width, UINT height, int frame_count, HWND hwnd);
    void RenderFrame();
};