#include "Renderer.h"

void Renderer::EnableDebugLayer() {
#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
    }
#endif
}

void Renderer::CreateGraphicsDevice(UINT width, UINT height, int frame_count) {
    width_ = width;
    height_ = height;
    frame_count_= frame_count;
    HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create graphics device");
    }
};

void Renderer::CreateFence() {
    HRESULT hr = device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create fence");
    }
};

void Renderer::AskDescryptorSizes() {
    rtv_descriptor_size_ =device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    dsv_descriptor_size_ =device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    cbv_srv_uav_descriptor_size_ =device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    sampler_descriptor_size_ =device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
};

//MSAA
/*
void Renderer::check4XMSAA() {
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQuality{};
    msaaQuality.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    msaaQuality.SampleCount = 4;
    HRESULT hr = device_->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQuality, sizeof(msaaQuality));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to check 4XMSAA");
    }
    if (msaaQuality.NumQualityLevels > 0) {
        sample_amount_ = 4;
        msaa_quality_ = msaaQuality.NumQualityLevels - 1;
    };
};
//mental disorder

void Renderer::CreateMSAARenderTarget() {
    //f*** old rt
    msaa_render_target_.Reset();
    D3D12_RESOURCE_DESC texDesc{};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = width_;
    texDesc.Height = height_;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = sample_amount_;
    texDesc.SampleDesc.Quality = msaa_quality_;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_CLEAR_VALUE optClear{};
    optClear.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    //background color
    optClear.Color[0] = 0.2f;
    optClear.Color[1] = 0.4f;
    optClear.Color[2] = 0.6f;
    optClear.Color[3] = 1.0f;
    D3D12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;
    HRESULT hr = device_->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &optClear, IID_PPV_ARGS(&msaa_render_target_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create MSAA render target");
    }
    // MSAA RTV
    msaa_rtv_handle_ = rtv_msaa_heap_->GetCPUDescriptorHandleForHeapStart();
    device_->CreateRenderTargetView(msaa_render_target_.Get(), nullptr, msaa_rtv_handle_);
}
*/
void Renderer::CreateCommandStuff() {
    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;
    HRESULT hr = device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&command_queue_));
    if (FAILED(hr)){
        throw std::runtime_error("Failed to create command queue");
    }
    hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create command allocator");
    }
    hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator_.Get(), nullptr, IID_PPV_ARGS(&command_list_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create command list");
    }
    hr = command_list_->Close();
    if (FAILED(hr)) { 
        throw std::runtime_error("Failed to close initial command list"); 
    }
};

void Renderer::CreateSwapChain(HWND hwnd){
    swap_chain_.Reset();
    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc{};
    swap_chain_desc.Width = width_;
    swap_chain_desc.Height = height_;
    swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    ComPtr<IDXGISwapChain1> tempSwapChain;
    HRESULT hr = factory->CreateSwapChainForHwnd(command_queue_.Get(), hwnd, &swap_chain_desc, nullptr, nullptr, &tempSwapChain);
    if (FAILED(hr))
        throw std::runtime_error("Failed to create SwapChain");
    tempSwapChain.As(&swap_chain_);
}

void Renderer::CreateHeaps(int textures_amount){
    HRESULT hr;
    // RTV HEAP
    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc{};
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    // swapchain buffers + gbuffer MRT
    rtvDesc.NumDescriptors = frame_count_ + 2;
    rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvDesc.NodeMask = 0;
    hr = device_->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&rtv_heap_));
    if (FAILED(hr)){
        throw std::runtime_error("Failed to create RTV heap");
    }
    // DSV HEAP
    D3D12_DESCRIPTOR_HEAP_DESC dsvDesc{};
    dsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvDesc.NumDescriptors = 1;
    dsvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvDesc.NodeMask = 0;
    hr = device_->CreateDescriptorHeap(&dsvDesc, IID_PPV_ARGS(&dsv_heap_));
    if (FAILED(hr)){
        throw std::runtime_error("Failed to create DSV heap");
    }
    // CBV / SRV / UAV HEAP
    D3D12_DESCRIPTOR_HEAP_DESC cbvDesc{};
    cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    // 2 CBV + 3 GBuffer SRV + textures
    cbvDesc.NumDescriptors = 2 + 3 + textures_amount;
    cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvDesc.NodeMask = 0;
    hr = device_->CreateDescriptorHeap(&cbvDesc, IID_PPV_ARGS(&cbv_srv_uav_heap_));
    if (FAILED(hr)){
        throw std::runtime_error("Failed to create CBV/SRV/UAV heap");
    }
    // SAMPLER HEAP
    D3D12_DESCRIPTOR_HEAP_DESC samplerDesc{};
    samplerDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerDesc.NumDescriptors = 1;
    samplerDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    samplerDesc.NodeMask = 0;
    hr = device_->CreateDescriptorHeap(&samplerDesc, IID_PPV_ARGS(&sampler_heap_));
    if (FAILED(hr)){
        throw std::runtime_error("Failed to create Sampler heap");
    }
}

void Renderer::CreateRenderingSystem()
{
    // RTV base
    D3D12_CPU_DESCRIPTOR_HANDLE rtvStart =rtv_heap_->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE gbufferRTV[2];
    // skip backbuffers
    rtvStart.ptr += frame_count_ * rtv_descriptor_size_;
    gbufferRTV[0] = rtvStart; // diffuse
    gbufferRTV[1].ptr =
        rtvStart.ptr + rtv_descriptor_size_; // normal
    // DSV
    D3D12_CPU_DESCRIPTOR_HANDLE gbufferDSV =dsv_heap_->GetCPUDescriptorHandleForHeapStart();
    // SRV heap
    D3D12_CPU_DESCRIPTOR_HANDLE srvStart =cbv_srv_uav_heap_->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE gbufferSRV[3];
    // skip CBV
    srvStart.ptr += 2 * cbv_srv_uav_descriptor_size_;
    gbufferSRV[0] = srvStart; // diffuse
    gbufferSRV[1].ptr =
        srvStart.ptr + cbv_srv_uav_descriptor_size_; // normal
    gbufferSRV[2].ptr =
        srvStart.ptr + 2 * cbv_srv_uav_descriptor_size_; // depth
    // Create gbuffer
    render_system_.InitGbuffer(device_.Get(),width_,height_,gbufferRTV,gbufferDSV,gbufferSRV);
}

void Renderer::CreateRTV() {
    render_targets_ = std::vector<ComPtr<ID3D12Resource>> (frame_count_);
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = rtv_heap_->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < frame_count_; i++){
        rtv_handle.ptr += SIZE_T(i) * rtv_descriptor_size_;
        HRESULT hr = swap_chain_->GetBuffer(i,IID_PPV_ARGS(&render_targets_[i]));
        if (FAILED(hr)){
            throw std::runtime_error("Failed to get swapchain buffer");
        }
        device_->CreateRenderTargetView(render_targets_[i].Get(),nullptr, rtv_handle);
    }
};

void Renderer::ViewportScissorSetup(){
    viewport_ = {};
    viewport_.TopLeftX = 0;
    viewport_.TopLeftY = 0;
    viewport_.Width = static_cast<float>(width_);
    viewport_.Height = static_cast<float>(height_);
    viewport_.MinDepth = 0.0f;
    viewport_.MaxDepth = 1.0f;
    scissor_rect_ = {};
    scissor_rect_.left = 0;
    scissor_rect_.top = 0;
    scissor_rect_.right = width_;
    scissor_rect_.bottom = height_;
}

void Renderer::LoadTextureFromTGA(TGAImage& image, UINT textureSlot){
    command_allocator_->Reset();
    command_list_->Reset(command_allocator_.Get(), nullptr);
    const UINT texWidth = image.get_width();
    const UINT texHeight = image.get_height();
    const UINT pixelSize = 4;

    //GPU-texture
    D3D12_RESOURCE_DESC texDesc{};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = texWidth;
    texDesc.Height = texHeight;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    D3D12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    HRESULT hr = device_->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&textures_[textureSlot])
    );
    if (FAILED(hr)){
        throw std::runtime_error("Failed to create texture resource");
    }
    //upload heap
    UINT64 uploadBufferSize;
    device_->GetCopyableFootprints(&texDesc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize);
    D3D12_RESOURCE_DESC uploadDesc{};
    uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    uploadDesc.Width = uploadBufferSize;
    uploadDesc.Height = 1;
    uploadDesc.DepthOrArraySize = 1;
    uploadDesc.MipLevels = 1;
    uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    uploadDesc.SampleDesc.Count = 1;
    D3D12_HEAP_PROPERTIES uploadHeapProps{};
    uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    ComPtr<ID3D12Resource> textureUploadHeap;
    hr = device_->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &uploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&textureUploadHeap)
    );
    if (FAILED(hr)){
        throw std::runtime_error("Failed to create texture upload heap");
    }
    //TGA â upload heap
    void* mappedData = nullptr;
    D3D12_RANGE readRange{ 0, 0 };
    textureUploadHeap->Map(0, &readRange, &mappedData);
    for (UINT y = 0; y < texHeight; y++){
        for (UINT x = 0; x < texWidth; x++){
            TGAColor color = image.get(x, y);
            UINT idx = (y * texWidth + x) * pixelSize;
            reinterpret_cast<BYTE*>(mappedData)[idx + 0] = color.b;
            reinterpret_cast<BYTE*>(mappedData)[idx + 1] = color.g;
            reinterpret_cast<BYTE*>(mappedData)[idx + 2] = color.r;
            reinterpret_cast<BYTE*>(mappedData)[idx + 3] = color.a;
        }
    }
    textureUploadHeap->Unmap(0, nullptr);
    // data to GPU texture
    D3D12_TEXTURE_COPY_LOCATION dst{};
    dst.pResource = textures_[textureSlot].Get();
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex = 0;
    D3D12_TEXTURE_COPY_LOCATION src{};
    src.pResource = textureUploadHeap.Get();
    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src.PlacedFootprint.Offset = 0;
    src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    src.PlacedFootprint.Footprint.Width = texWidth;
    src.PlacedFootprint.Footprint.Height = texHeight;
    src.PlacedFootprint.Footprint.Depth = 1;
    src.PlacedFootprint.Footprint.RowPitch = texWidth * pixelSize;
    command_list_->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
    //Barrier: texture to PIXEL_SHADER_RESOURCE
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = textures_[textureSlot].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    command_list_->ResourceBarrier(1, &barrier);
    // make SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    D3D12_CPU_DESCRIPTOR_HANDLE handle = cbv_srv_uav_heap_->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += (textureSlot+5) * cbv_srv_uav_descriptor_size_;
    device_->CreateShaderResourceView(textures_[textureSlot].Get(), &srvDesc, handle);
    command_list_->Close();
    ID3D12CommandList* lists[] = { command_list_.Get() };
    command_queue_->ExecuteCommandLists(1, lists);
    UINT64 fenceValue = ++fence_value_;
    command_queue_->Signal(fence_.Get(), fenceValue);
    if (fence_->GetCompletedValue() < fenceValue) {
        HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        fence_->SetEventOnCompletion(fenceValue, eventHandle);
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

void Renderer::CreateCBV_SRV_Sampler(XMVECTOR cam_pos,XMVECTOR look_at,XMVECTOR up,Model mesh,XMFLOAT3 light_pos){
    HRESULT hr;
    // Heap properties
    D3D12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;
    D3D12_RESOURCE_DESC bufferDesc{};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment = 0;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.SampleDesc.Quality = 0;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    // CBV : CbPerPass  (slot 0)

    bufferDesc.Width = (sizeof(CbPerPass) + 255) & ~255;

    hr = device_->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&cb_perpass_)
    );

    if (FAILED(hr))
        throw std::runtime_error("Failed to create CbPerPass");

    cb_perpass_->Map(0, nullptr, &cb_perpass_mapped_);

    D3D12_CONSTANT_BUFFER_VIEW_DESC perPassDesc{};
    perPassDesc.BufferLocation = cb_perpass_->GetGPUVirtualAddress();
    perPassDesc.SizeInBytes = bufferDesc.Width;

    D3D12_CPU_DESCRIPTOR_HANDLE handle =
        cbv_srv_uav_heap_->GetCPUDescriptorHandleForHeapStart();

    device_->CreateConstantBufferView(&perPassDesc, handle);
    // CBV : CbLight (slot 1)
    bufferDesc.Width = (sizeof(CbLight) + 255) & ~255;

    hr = device_->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&cb_light_)
    );

    if (FAILED(hr))
        throw std::runtime_error("Failed to create CbLight");

    cb_light_->Map(0, nullptr, &cb_light_mapped_);

    D3D12_CONSTANT_BUFFER_VIEW_DESC lightDesc{};
    lightDesc.BufferLocation = cb_light_->GetGPUVirtualAddress();
    lightDesc.SizeInBytes = bufferDesc.Width;

    handle.ptr += cbv_srv_uav_descriptor_size_;

    device_->CreateConstantBufferView(&lightDesc, handle);
    // MATERIALS
    UINT materialCount = (UINT)mesh.GetMaterials().size();
    material_cb_.resize(materialCount);
    material_cb_mapped_.resize(materialCount);

    textures_.resize(materialCount);

    for (UINT i = 0; i < materialCount; i++){
        // CBV : CbMaterial
        // slot = 5 + i*2
        bufferDesc.Width = (sizeof(CbMaterial) + 255) & ~255;
        hr = device_->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&material_cb_[i])
        );
        if (FAILED(hr)){
            throw std::runtime_error("Failed to create material CB");
        }
        material_cb_[i]->Map(0, nullptr, &material_cb_mapped_[i]);
        D3D12_CONSTANT_BUFFER_VIEW_DESC matDesc{};
        matDesc.BufferLocation = material_cb_[i]->GetGPUVirtualAddress();
        matDesc.SizeInBytes = bufferDesc.Width;
        UINT index = 5 + i * 2;
        handle = cbv_srv_uav_heap_->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += index * cbv_srv_uav_descriptor_size_;
        device_->CreateConstantBufferView(&matDesc, handle);
        // SRV : material texture slot = 6 + i*2
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Shader4ComponentMapping =D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        handle = cbv_srv_uav_heap_->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += (index + 1) * cbv_srv_uav_descriptor_size_;
        device_->CreateShaderResourceView(textures_[i].Get(),&srvDesc,handle);
        // initial material data
        CbMaterial matData{};
        matData.time = 0.0f;

        memcpy(material_cb_mapped_[i], &matData, sizeof(CbMaterial));
    }
    // SAMPLER
    D3D12_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    D3D12_CPU_DESCRIPTOR_HANDLE sampHandle =sampler_heap_->GetCPUDescriptorHandleForHeapStart();
    device_->CreateSampler(&samplerDesc, sampHandle);
    // INITIAL DATA
    CbPerPass perPassData{};
    XMStoreFloat4x4(&perPassData.model, XMMatrixIdentity());
    XMStoreFloat4x4(&perPassData.inv_model,
    XMMatrixInverse(nullptr, XMMatrixIdentity()));
    XMMATRIX view = XMMatrixLookAtLH(cam_pos, look_at, up);
    XMStoreFloat4x4(&perPassData.view, view);
    XMStoreFloat4x4(&perPassData.inv_view,
    XMMatrixInverse(nullptr, view));
    XMMATRIX proj =XMMatrixPerspectiveFovLH(XM_PIDIV4,float(width_) / float(height_),0.1f,10000.0f);
    XMStoreFloat4x4(&perPassData.projection, proj);
    XMStoreFloat4x4(&perPassData.inv_projection,XMMatrixInverse(nullptr, proj));
    XMStoreFloat3(&perPassData.cameraPos, cam_pos);
    perPassData.nearZ = 0.1f;
    perPassData.farZ = 10000.0f;
    perPassData.time = 0.0f;
    memcpy(cb_perpass_mapped_, &perPassData, sizeof(CbPerPass));
    CbLight lightData{};
    lightData.lightPos = light_pos;
    lightData.lightDir = XMFLOAT3(0, -1, 0);
    lightData.lightType = 1;
    lightData.ambient_k = XMFLOAT4(0.1f, 0.1f, 0.1f, 1);
    lightData.diffuse_k = XMFLOAT4(1, 1, 1, 1);
    lightData.specular_k = XMFLOAT4(1, 1, 1, 1);
    lightData.intensity = 5.0f;
    lightData.range = 50.0f;
    lightData.shiny_k = 32;
    lightData.screenSize =XMFLOAT2((float)width_, (float)height_);
    memcpy(cb_light_mapped_, &lightData, sizeof(CbLight));
}
void Renderer::CreateVertexBuffer(Model& model){
    const std::vector<Vertex>& vertices = model.GetVertices();
    vertex_count_ = vertices.size();
    UINT bufferSize = vertex_count_ * sizeof(Vertex);
    D3D12_RESOURCE_DESC bufferDesc{};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = bufferSize;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    D3D12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    HRESULT hr=device_->CreateCommittedResource(&heapProps,D3D12_HEAP_FLAG_NONE,&bufferDesc,D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&vertex_buffer_));
    if (FAILED(hr))
        throw std::runtime_error("Failed to create vertex buffer");
    void* mappedData = nullptr;
    D3D12_RANGE readRange{ 0, 0 };
    hr=vertex_buffer_->Map(0, &readRange, &mappedData);
    if (FAILED(hr))
        throw std::runtime_error("Failed to fill vertex buffer");
    memcpy(mappedData, vertices.data(), bufferSize);
    vertex_buffer_->Unmap(0, nullptr);
    vertex_buffer_view_.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
    vertex_buffer_view_.StrideInBytes = sizeof(Vertex);
    vertex_buffer_view_.SizeInBytes = bufferSize;
}


void Renderer::Initialize(UINT width, UINT height, int frame_count, HWND hwnd, Model& mesh, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, XMFLOAT3 light_pos){
    dummy_.read_tga_file("dummy.tga");
    //EnableDebugLayer();
    frame_fence_values_.resize(frame_count, 0);
    CreateGraphicsDevice(width, height, frame_count);
    CreateFence();
    AskDescryptorSizes();
    CreateHeaps(mesh.GetMaterials().size());
    CreateCommandStuff();
    CreateSwapChain(hwnd);
    CreateRTV();
    ViewportScissorSetup();
    CreateRenderingSystem();
    CreateCBV_SRV_Sampler(cam_pos, look_at, up, mesh,light_pos);
    for (int i = 0; i < mesh.GetMaterials().size(); i++) {
        TGAImage image = mesh.GetMaterials()[i].diffuseTexture;
       if (not(mesh.GetMaterials()[i].hasDiffuseTexture)) {
            image = dummy_; }
       if (image.get_height()==0 or image.get_width() == 0) {
           image = dummy_;
           OutputDebugStringA(mesh.GetMaterials()[i].diffuseTexPath.c_str());
       }
        LoadTextureFromTGA(image, i);
    }
    CreateVertexBuffer(mesh);

}
void Renderer::RenderFrame(Model& mesh, float time, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up) {
    //Reset
    command_allocator_->Reset();
    command_list_->Reset(command_allocator_.Get(), nullptr);
    //resourse barriers
    D3D12_RESOURCE_BARRIER barriersBegin[2];
    UINT barrierCount = 0;
    //back buffer: PRESENT -> RESOLVE_DEST
    barriersBegin[barrierCount++] = {
        D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        D3D12_RESOURCE_BARRIER_FLAG_NONE,
        {
            render_targets_[current_backbuffer_].Get(),
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RESOLVE_DEST
        }
    };
    command_list_->ResourceBarrier(barrierCount, barriersBegin);
    //Viewport / Scissor
    command_list_->RSSetViewports(1, &viewport_);
    command_list_->RSSetScissorRects(1, &scissor_rect_);
    //RTV / DSV (MSAA rt instead of back buffer)
    command_list_->OMSetRenderTargets(1, &msaa_rtv_handle_, FALSE, &dsvHandle_);
    //Clear MSAA rt and DSV
    const float clearColor[] = { 0.2f, 0.4f, 0.6f, 1.0f };
    //command_list_->ClearRenderTargetView(msaa_rtv_handle_, clearColor, 0, nullptr);
    //command_list_->ClearDepthStencilView(dsvHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    
    render_system_.RenderFrame(command_list_.Get(), mesh);

    //barrier back buffer RESOLVE_DEST -> PRESENT
    D3D12_RESOURCE_BARRIER postResolveBarrier = {
        D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        D3D12_RESOURCE_BARRIER_FLAG_NONE,
        {
            render_targets_[current_backbuffer_].Get(),
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            D3D12_RESOURCE_STATE_RESOLVE_DEST,
            D3D12_RESOURCE_STATE_PRESENT
        }
    };
    command_list_->ResourceBarrier(1, &postResolveBarrier);

    //Close Execute
    command_list_->Close();
    ID3D12CommandList* lists[] = { command_list_.Get() };
    command_queue_->ExecuteCommandLists(1, lists);

    //Present
    swap_chain_->Present(1, 0);
    current_backbuffer_ = swap_chain_->GetCurrentBackBufferIndex();

    //Fence
    UINT64 fenceValue = ++fence_value_;
    command_queue_->Signal(fence_.Get(), fenceValue);
    frame_fence_values_[current_backbuffer_] = fenceValue;
    if (fence_->GetCompletedValue() < frame_fence_values_[current_backbuffer_]) {
        fence_->SetEventOnCompletion(frame_fence_values_[current_backbuffer_], eventHandle_);
        WaitForSingleObject(eventHandle_, INFINITE);
       // CloseHandle(eventHandle_);
    }

    first_frame_ = false;
}