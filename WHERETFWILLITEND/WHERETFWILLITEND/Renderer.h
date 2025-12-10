#pragma once
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

class Renderer
{
private:
        Microsoft::WRL::ComPtr<ID3D12Device> device_;
        Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue_;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator_;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list_;
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
        ComPtr<ID3D12DescriptorHeap> rtv_heap_;
        UINT rtv_descriptor_size_;
        ComPtr<ID3D12DescriptorHeap> dsv_heap_;
        UINT dsv_descriptor_size_;
        ComPtr<ID3D12DescriptorHeap> cbv_srv_uav_heap_;
        UINT cbv_srv_uav_descriptor_size_;
        ComPtr<ID3D12DescriptorHeap> sampler_heap_;
        UINT sampler_descriptor_size_;
public:
    // step2
    HRESULT CreateGraphicsDevice();
    // step3.1
    void CreateFence();
    // step3.2
    void AskDescryptorSizes();
    // step4
    // step5
    // step6
    // step7
    // step8
    // step9
    // step10
};
