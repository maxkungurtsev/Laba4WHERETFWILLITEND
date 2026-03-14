#include "G_buffer.h"

using Microsoft::WRL::ComPtr;

void GBuffer::Create(
    ID3D12Device* device,
    uint32_t width,
    uint32_t height,
    const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles,
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,
    const D3D12_CPU_DESCRIPTOR_HANDLE* srvHandles)
{
    width_ = width;
    height_ = height;

    rtvs_[0] = rtvHandles[0];
    rtvs_[1] = rtvHandles[1];

    srv_albedo_ = srvHandles[0];
    srv_normal_ = srvHandles[1];
    srv_depth_ = srvHandles[2];

    dsv_ = dsvHandle;

    HRESULT hr;
    // HEAP PROPERTIES
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;
    // ALBEDO BUFFER
    D3D12_RESOURCE_DESC albedoDesc = {};
    albedoDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    albedoDesc.Alignment = 0;
    albedoDesc.Width = width;
    albedoDesc.Height = height;
    albedoDesc.DepthOrArraySize = 1;
    albedoDesc.MipLevels = 1;
    albedoDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    albedoDesc.SampleDesc.Count = 1;
    albedoDesc.SampleDesc.Quality = 0;
    albedoDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    albedoDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearAlbedo = {};
    clearAlbedo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    clearAlbedo.Color[0] = 0.0f;
    clearAlbedo.Color[1] = 0.0f;
    clearAlbedo.Color[2] = 0.0f;
    clearAlbedo.Color[3] = 1.0f;

    hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &albedoDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        &clearAlbedo,
        IID_PPV_ARGS(&albedo_));

    if (FAILED(hr))
        throw std::runtime_error("Failed to create GBuffer Albedo");

    device->CreateRenderTargetView(albedo_.Get(), nullptr, rtvs_[0]);
    device->CreateShaderResourceView(albedo_.Get(), nullptr, srv_albedo_);
    // NORMAL BUFFER
    D3D12_RESOURCE_DESC normalDesc = albedoDesc;
    normalDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

    D3D12_CLEAR_VALUE clearNormal = {};
    clearNormal.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    clearNormal.Color[0] = 0.0f;
    clearNormal.Color[1] = 0.0f;
    clearNormal.Color[2] = 0.0f;
    clearNormal.Color[3] = 1.0f;

    hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &normalDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        &clearNormal,
        IID_PPV_ARGS(&normal_));

    if (FAILED(hr))
        throw std::runtime_error("Failed to create GBuffer Normal");

    device->CreateRenderTargetView(normal_.Get(), nullptr, rtvs_[1]);
    device->CreateShaderResourceView(normal_.Get(), nullptr, srv_normal_);
    // DEPTH BUFFER
    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Alignment = 0;
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearDepth = {};
    clearDepth.Format = DXGI_FORMAT_D32_FLOAT;
    clearDepth.DepthStencil.Depth = 1.0f;
    clearDepth.DepthStencil.Stencil = 0;

    hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearDepth,
        IID_PPV_ARGS(&depth_));

    if (FAILED(hr))
        throw std::runtime_error("Failed to create GBuffer Depth");
    // DEPTH DSV
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    device->CreateDepthStencilView(depth_.Get(), &dsvDesc, dsv_);
    // DEPTH SRV
    // =========================

    D3D12_SHADER_RESOURCE_VIEW_DESC depthSRV = {};
    depthSRV.Format = DXGI_FORMAT_R32_FLOAT;
    depthSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    depthSRV.Texture2D.MipLevels = 1;
    depthSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    device->CreateShaderResourceView(depth_.Get(), &depthSRV, srv_depth_);
}

void GBuffer::BindForGeometryPass(ID3D12GraphicsCommandList* cmdList)
{
    // Depth -> writable

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = depth_.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    cmdList->ResourceBarrier(1, &barrier);

    cmdList->OMSetRenderTargets(
        2,
        rtvs_,
        FALSE,
        &dsv_);

    float clearColor[4] = { 0,0,0,1 };

    cmdList->ClearRenderTargetView(rtvs_[0], clearColor, 0, nullptr);
    cmdList->ClearRenderTargetView(rtvs_[1], clearColor, 0, nullptr);

    cmdList->ClearDepthStencilView(
        dsv_,
        D3D12_CLEAR_FLAG_DEPTH,
        1.0f,
        0,
        0,
        nullptr);
}

void GBuffer::BindForLightingPass(
    ID3D12GraphicsCommandList* cmdList,
    D3D12_GPU_DESCRIPTOR_HANDLE gbufferSrvTable)
{
    // Depth -> readable

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = depth_.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    cmdList->ResourceBarrier(1, &barrier);

    cmdList->SetGraphicsRootDescriptorTable(
        2,
        gbufferSrvTable);
}