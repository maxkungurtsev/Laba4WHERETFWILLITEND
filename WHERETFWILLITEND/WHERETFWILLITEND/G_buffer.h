#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <windows.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <iostream>
#include <vector>
#include <sstream>
using Microsoft::WRL::ComPtr;
class GBuffer
{
public:
    void Create(ID3D12Device* device,uint32_t width,uint32_t height,
        const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles,
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,
        const D3D12_CPU_DESCRIPTOR_HANDLE* srvHandles);
    void BindForGeometryPass(ID3D12GraphicsCommandList* cmdList);
    void BindForLightingPass(
        ID3D12GraphicsCommandList* cmdList,
        D3D12_GPU_DESCRIPTOR_HANDLE gbufferSrvTable);

private:
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    ComPtr<ID3D12Resource> albedo_;
    ComPtr<ID3D12Resource> normal_;
    ComPtr<ID3D12Resource> depth_;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvs_[2]{};
    D3D12_CPU_DESCRIPTOR_HANDLE srv_albedo_{};
    D3D12_CPU_DESCRIPTOR_HANDLE srv_normal_{};
    D3D12_CPU_DESCRIPTOR_HANDLE srv_depth_{};
    D3D12_CPU_DESCRIPTOR_HANDLE dsv_{};
};