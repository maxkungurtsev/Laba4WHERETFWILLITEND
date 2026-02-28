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
    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.NumDescriptors = frame_count_;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;
    HRESULT hr = device_->CreateDescriptorHeap(&desc,IID_PPV_ARGS(&rtv_heap_));
    if (FAILED(hr)){
        throw std::runtime_error("Failed to create RTV heap");
    }
    desc.NumDescriptors = 1;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    hr = device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dsv_heap_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create DSV heap");
    }
    D3D12_DESCRIPTOR_HEAP_DESC cbvDesc{};
    cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvDesc.NumDescriptors = 2+ textures_amount;
    cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    hr = device_->CreateDescriptorHeap(&cbvDesc, IID_PPV_ARGS(&cbv_srv_uav_heap_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create CBV, SRV and UAV heap");
    }
    D3D12_DESCRIPTOR_HEAP_DESC sampDesc{};
    sampDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    sampDesc.NumDescriptors = 1;
    sampDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    hr=device_->CreateDescriptorHeap(&sampDesc, IID_PPV_ARGS(&sampler_heap_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create Sampler heap");
    }
    D3D12_DESCRIPTOR_HEAP_DESC msaaRtvDesc{};
    msaaRtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    msaaRtvDesc.NumDescriptors = 1; // 1 RTV for MSAA RT
    msaaRtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    msaaRtvDesc.NodeMask = 0;
    hr = device_->CreateDescriptorHeap(&msaaRtvDesc, IID_PPV_ARGS(&rtv_msaa_heap_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create MSAA RTV heap");
    }
};

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

void Renderer::CreateZBuffer(){
    //resourse desc
    D3D12_RESOURCE_DESC depthDesc{};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = width_;
    depthDesc.Height = height_;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = sample_amount_;
    depthDesc.SampleDesc.Quality = msaa_quality_;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    //ClearValue
    D3D12_CLEAR_VALUE optClear{};
    optClear.Format = DXGI_FORMAT_D32_FLOAT;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    //heap props
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;
    //resourse
    HRESULT hr=device_->CreateCommittedResource(&heapProps,D3D12_HEAP_FLAG_NONE,&depthDesc,D3D12_RESOURCE_STATE_DEPTH_WRITE,&optClear,IID_PPV_ARGS(&z_buffer_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create dsv resourse");
    }
    //DSV
    device_->CreateDepthStencilView(z_buffer_.Get(), nullptr, dsv_heap_->GetCPUDescriptorHandleForHeapStart());
}

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
    handle.ptr += (textureSlot+2) * cbv_srv_uav_descriptor_size_;
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

void Renderer::CreateCBV_SRV_Sampler(XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, Model mesh, XMFLOAT3 light_pos){
    //CBV MVP
    D3D12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resDesc{};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Alignment = 0;
    resDesc.Width = (sizeof(MVPConstants) + 255) & ~255;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count = 1;
    resDesc.SampleDesc.Quality = 0;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    //resourse MVP CBV
    HRESULT hr = device_->CreateCommittedResource(&heapProps,D3D12_HEAP_FLAG_NONE,&resDesc,D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&mvp_cb_));
    if (FAILED(hr)){
         throw std::runtime_error("Failed to create MVP CBV resource");
    }
    // Map
    mvp_cb_->Map(0, nullptr, &mvp_cb_mapped_);
    //CBV desc
    D3D12_CONSTANT_BUFFER_VIEW_DESC mvp_cbv_desc{};
    mvp_cbv_desc.BufferLocation = mvp_cb_->GetGPUVirtualAddress();
    mvp_cbv_desc.SizeInBytes = resDesc.Width;
    D3D12_CPU_DESCRIPTOR_HANDLE handle = cbv_srv_uav_heap_->GetCPUDescriptorHandleForHeapStart();
    device_->CreateConstantBufferView(&mvp_cbv_desc, handle);
    //CBV: Light
    resDesc.Width = (sizeof(LightConstants) + 255) & ~255;
    hr = device_->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&light_cb_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create Light CBV resource");
    }
    light_cb_->Map(0, nullptr, &light_cb_mapped_);
    D3D12_CONSTANT_BUFFER_VIEW_DESC light_cbv_desc{};
    light_cbv_desc.BufferLocation = light_cb_->GetGPUVirtualAddress();
    light_cbv_desc.SizeInBytes = resDesc.Width;
    handle.ptr += cbv_srv_uav_descriptor_size_;
    device_->CreateConstantBufferView(&light_cbv_desc, handle);
    //SRV Texture
    textures_.resize(mesh.GetMaterials().size());
    for (int i = 0; i < mesh.GetMaterials().size(); i++) {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        handle = cbv_srv_uav_heap_->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += (2+i) * cbv_srv_uav_descriptor_size_;
    }

    //Sampler
    D3D12_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    D3D12_CPU_DESCRIPTOR_HANDLE sampHandle = sampler_heap_->GetCPUDescriptorHandleForHeapStart();
    device_->CreateSampler(&samplerDesc, sampHandle);
    // filling up MVP
    MVPConstants mvpData{};
    // model = identity
    XMStoreFloat4x4(&mvpData.model, XMMatrixIdentity());
    // view
    XMStoreFloat4x4(&mvpData.view,XMMatrixLookAtLH(cam_pos, look_at, up));
    // projection
    XMStoreFloat4x4(&mvpData.projection,XMMatrixPerspectiveFovLH(XM_PIDIV4,float(width_) / float(height_),0.1f,1000.0f));
    memcpy(mvp_cb_mapped_, &mvpData, sizeof(MVPConstants));
    //filling up light
    LightConstants lightData{};
    XMFLOAT3 camera_position;
    XMStoreFloat3(&camera_position, cam_pos);
    lightData.lightPos = light_pos;
    lightData.cameraPos = camera_position;
    lightData.ambient_k = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f);
    lightData.diffuse_k = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.3f);
    lightData.specular_k = XMFLOAT4(0.8f, 0.8f, 0.8f, 0.8f);
    lightData.shiny_k = 32;
    lightData.intensity = 10;
    lightData.pad3[0] = lightData.pad3[1] = lightData.pad3[2] = 0.0f;
    lightData.time = 0.0;
    memcpy(light_cb_mapped_, &lightData, sizeof(LightConstants));
}

void Renderer::CreateRootSignature(int textures_amount) {
    // CBV b0 (MVP) - VS
    D3D12_DESCRIPTOR_RANGE1 cbvRangeVS{};
    cbvRangeVS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    cbvRangeVS.NumDescriptors = 1;
    cbvRangeVS.BaseShaderRegister = 0; // b0
    cbvRangeVS.RegisterSpace = 0;
    cbvRangeVS.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
    cbvRangeVS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    // CBV b1 (Light) - PS
    D3D12_DESCRIPTOR_RANGE1 cbvRangePS{};
    cbvRangePS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    cbvRangePS.NumDescriptors = 1;
    cbvRangePS.BaseShaderRegister = 1; // b1
    cbvRangePS.RegisterSpace = 0;
    cbvRangePS.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
    cbvRangePS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    // SRV t0 (diffuseMap) - PS
    D3D12_DESCRIPTOR_RANGE1 srvRange{};
    srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRange.NumDescriptors = textures_amount;
    srvRange.BaseShaderRegister = 0; // t0
    srvRange.RegisterSpace = 0;
    srvRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
    srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    // Sampler s0 - PS
    D3D12_DESCRIPTOR_RANGE1 samplerRange{};
    samplerRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    samplerRange.NumDescriptors = 1;
    samplerRange.BaseShaderRegister = 0; // s0
    samplerRange.RegisterSpace = 0;
    samplerRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
    samplerRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    // Root parameters
    D3D12_ROOT_PARAMETER1 rootParams[4]{};
    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[0].DescriptorTable.pDescriptorRanges = &cbvRangeVS;
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[1].DescriptorTable.pDescriptorRanges = &cbvRangePS;
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[2].DescriptorTable.pDescriptorRanges = &srvRange;
    rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParams[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[3].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[3].DescriptorTable.pDescriptorRanges = &samplerRange;
    rootParams[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    // Root signature
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    rootSigDesc.Desc_1_1.NumParameters = _countof(rootParams);
    rootSigDesc.Desc_1_1.pParameters = rootParams;
    rootSigDesc.Desc_1_1.NumStaticSamplers = 0;
    rootSigDesc.Desc_1_1.pStaticSamplers = nullptr;
    rootSigDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    ComPtr<ID3DBlob> serialized;
    ComPtr<ID3DBlob> error;
    HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSigDesc, &serialized, &error);
    if (FAILED(hr)) {
        if (error) OutputDebugStringA((char*)error->GetBufferPointer());
        throw std::runtime_error("Failed to serialize root signature");
    }
    hr = device_->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&root_signature_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create root signature");
    }
}

void Renderer::CompileShaders() {
    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(L"VertexShader.hlsl",nullptr, nullptr,"main", "vs_5_0",0, 0,&vertex_shader_,&errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "Shader compile error: ";
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        else {
            OutputDebugStringA("Shader compile failed, but no error message was produced.");
        }
        throw std::runtime_error("Failed to compile vertex shader");
    }
    hr = D3DCompileFromFile(L"VertexShader_anim_.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertex_shader_anim_, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "Shader compile error: ";
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        else {
            OutputDebugStringA("Shader compile failed, but no error message was produced.");
        }
        throw std::runtime_error("Failed to compile vertex shader with animation");
    }
    hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixel_shader_, &errorBlob);
    if (FAILED(hr)){
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        throw std::runtime_error("Failed to compile pixel shader");
    }
};

void Renderer::CreatePipelineStateObject() {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { input_layout_.data(), (UINT)input_layout_.size() };
    psoDesc.pRootSignature = root_signature_.Get();
    psoDesc.VS = { vertex_shader_->GetBufferPointer(), vertex_shader_->GetBufferSize() };
    psoDesc.PS = { pixel_shader_->GetBufferPointer(), pixel_shader_->GetBufferSize() };
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
    psoDesc.BlendState.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC defaultBlend = {FALSE,FALSE,
        D3D12_BLEND_ONE,D3D12_BLEND_ZERO,D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE,D3D12_BLEND_ZERO,D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL
    };
    for (int i = 0; i < 8; ++i){
        psoDesc.BlendState.RenderTarget[i] = defaultBlend;
    }
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = sample_amount_;
    psoDesc.SampleDesc.Quality = msaa_quality_;
    //RasterizerState
    D3D12_RASTERIZER_DESC rasterDesc{};
    rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.MultisampleEnable = FALSE;
    rasterDesc.AntialiasedLineEnable = FALSE;
    rasterDesc.ForcedSampleCount = 0;
    rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    psoDesc.RasterizerState = rasterDesc;
    //blend state
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    D3D12_RENDER_TARGET_BLEND_DESC rtBlend{};
    rtBlend.BlendEnable = FALSE;
    rtBlend.LogicOpEnable = FALSE;
    rtBlend.SrcBlend = D3D12_BLEND_ONE;
    rtBlend.DestBlend = D3D12_BLEND_ZERO;
    rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
    rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
    rtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
    rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rtBlend.LogicOp = D3D12_LOGIC_OP_NOOP;
    rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    for (int i = 0; i < 8; ++i){
            blendDesc.RenderTarget[i] = rtBlend;
    }
    psoDesc.BlendState = blendDesc;
    //DepthStencilState
    D3D12_DEPTH_STENCIL_DESC depthDesc{};
    depthDesc.DepthEnable = TRUE;
    depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthDesc.StencilEnable = FALSE;
    depthDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depthDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    depthDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    depthDesc.BackFace = depthDesc.FrontFace;
    psoDesc.DepthStencilState = depthDesc;
    // D E B U G  T I M E
    if (!root_signature_) OutputDebugStringA("root_signature_ == null\n");
    if (!vertex_shader_) OutputDebugStringA("vertex_shader_ == null\n");
    if (!pixel_shader_) OutputDebugStringA("pixel_shader_ == null\n");
    {
        std::ostringstream ss;
        ss << "VS size: " << (vertex_shader_ ? vertex_shader_->GetBufferSize() : 0)
            << ", PS size: " << (pixel_shader_ ? pixel_shader_->GetBufferSize() : 0) << "\n";
        OutputDebugStringA(ss.str().c_str());
    }
    if (input_layout_.empty()) OutputDebugStringA("input_layout_ empty\n");
    {
        std::ostringstream s2;
        s2 << "RTVFormat: " << psoDesc.RTVFormats[0] << " DSVFormat: " << psoDesc.DSVFormat << " SampleCount: " << psoDesc.SampleDesc.Count << "\n";
        OutputDebugStringA(s2.str().c_str());
    }
    std::ostringstream oss;
    oss << "Creating PSO with parameters:\n";
    oss << "NumRenderTargets: " << psoDesc.NumRenderTargets << "\n";
    oss << "RTVFormats[0]: " << psoDesc.RTVFormats[0] << "\n";
    oss << "DSVFormat: " << psoDesc.DSVFormat << "\n";
    oss << "SampleCount: " << psoDesc.SampleDesc.Count << "\n";
    oss << "InputLayout.Elements: " << psoDesc.InputLayout.NumElements << "\n";
    oss << "RootSignature: " << (psoDesc.pRootSignature ? "valid" : "nullptr") << "\n";
    oss << "VS Size: " << psoDesc.VS.BytecodeLength << "\n";
    oss << "PS Size: " << psoDesc.PS.BytecodeLength << "\n";
    OutputDebugStringA(oss.str().c_str());

    HRESULT hr = device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipeline_state_));
    if (FAILED(hr)) {
        std::ostringstream oss2;
        oss2 << "CreateGraphicsPipelineState failed. HRESULT = 0x" << std::hex << hr << "\n";
        OutputDebugStringA(oss2.str().c_str());
        throw std::runtime_error(oss2.str());
    }
};
void Renderer::CreatePipelineStateObjectAnim() {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { input_layout_.data(), (UINT)input_layout_.size() };
    psoDesc.pRootSignature = root_signature_.Get();
    psoDesc.VS = { vertex_shader_anim_->GetBufferPointer(), vertex_shader_anim_->GetBufferSize() };
    psoDesc.PS = { pixel_shader_->GetBufferPointer(), pixel_shader_->GetBufferSize() };
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
    psoDesc.BlendState.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC defaultBlend = { FALSE,FALSE,
        D3D12_BLEND_ONE,D3D12_BLEND_ZERO,D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE,D3D12_BLEND_ZERO,D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL
    };
    for (int i = 0; i < 8; ++i) {
        psoDesc.BlendState.RenderTarget[i] = defaultBlend;
    }
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = sample_amount_;
    psoDesc.SampleDesc.Quality = msaa_quality_;
    //RasterizerState
    D3D12_RASTERIZER_DESC rasterDesc{};
    rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.MultisampleEnable = FALSE;
    rasterDesc.AntialiasedLineEnable = FALSE;
    rasterDesc.ForcedSampleCount = 0;
    rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    psoDesc.RasterizerState = rasterDesc;
    //blend state
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    D3D12_RENDER_TARGET_BLEND_DESC rtBlend{};
    rtBlend.BlendEnable = FALSE;
    rtBlend.LogicOpEnable = FALSE;
    rtBlend.SrcBlend = D3D12_BLEND_ONE;
    rtBlend.DestBlend = D3D12_BLEND_ZERO;
    rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
    rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
    rtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
    rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rtBlend.LogicOp = D3D12_LOGIC_OP_NOOP;
    rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    for (int i = 0; i < 8; ++i) {
        blendDesc.RenderTarget[i] = rtBlend;
    }
    psoDesc.BlendState = blendDesc;
    //DepthStencilState
    D3D12_DEPTH_STENCIL_DESC depthDesc{};
    depthDesc.DepthEnable = TRUE;
    depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthDesc.StencilEnable = FALSE;
    depthDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depthDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    depthDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    depthDesc.BackFace = depthDesc.FrontFace;
    psoDesc.DepthStencilState = depthDesc;
    // D E B U G  T I M E
    if (!root_signature_) OutputDebugStringA("root_signature_ == null\n");
    if (!vertex_shader_anim_) OutputDebugStringA("vertex_shader_ == null\n");
    if (!pixel_shader_) OutputDebugStringA("pixel_shader_ == null\n");
    {
        std::ostringstream ss;
        ss << "VS size: " << (vertex_shader_anim_ ? vertex_shader_anim_->GetBufferSize() : 0)
            << ", PS size: " << (pixel_shader_ ? pixel_shader_->GetBufferSize() : 0) << "\n";
        OutputDebugStringA(ss.str().c_str());
    }
    if (input_layout_.empty()) OutputDebugStringA("input_layout_ empty\n");
    {
        std::ostringstream s2;
        s2 << "RTVFormat: " << psoDesc.RTVFormats[0] << " DSVFormat: " << psoDesc.DSVFormat << " SampleCount: " << psoDesc.SampleDesc.Count << "\n";
        OutputDebugStringA(s2.str().c_str());
    }
    std::ostringstream oss;
    oss << "Creating PSO with parameters:\n";
    oss << "NumRenderTargets: " << psoDesc.NumRenderTargets << "\n";
    oss << "RTVFormats[0]: " << psoDesc.RTVFormats[0] << "\n";
    oss << "DSVFormat: " << psoDesc.DSVFormat << "\n";
    oss << "SampleCount: " << psoDesc.SampleDesc.Count << "\n";
    oss << "InputLayout.Elements: " << psoDesc.InputLayout.NumElements << "\n";
    oss << "RootSignature: " << (psoDesc.pRootSignature ? "valid" : "nullptr") << "\n";
    oss << "VS Size: " << psoDesc.VS.BytecodeLength << "\n";
    oss << "PS Size: " << psoDesc.PS.BytecodeLength << "\n";
    OutputDebugStringA(oss.str().c_str());

    HRESULT hr = device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipeline_state_anim_));
    if (FAILED(hr)) {
        std::ostringstream oss2;
        oss2 << "CreateGraphicsPipelineState failed. HRESULT = 0x" << std::hex << hr << "\n";
        OutputDebugStringA(oss2.str().c_str());
        throw std::runtime_error(oss2.str());
    }
};
///////////////
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

void Renderer::CreateInputLayout(){
    input_layout_ =
    {
        {
            "POSITION",
            0, 
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            0, 
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "NORMAL",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            12,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "TEXCOORD",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0,
            24,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        }
    };
}


void Renderer::Initialize(UINT width, UINT height, int frame_count, HWND hwnd, Model& mesh, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, XMFLOAT3 light_pos){
    dummy_.read_tga_file("dummy.tga");
    EnableDebugLayer();
    CreateGraphicsDevice(width, height, frame_count);
    CreateFence();
    check4XMSAA();
    AskDescryptorSizes();
    CreateHeaps(mesh.GetMaterials().size());
    CreateCommandStuff();
    CreateSwapChain(hwnd);
    CreateRTV();
    CreateMSAARenderTarget();
    CreateZBuffer();
    ViewportScissorSetup();
    CreateRootSignature(mesh.GetMaterials().size());
    CreateInputLayout();
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
    CompileShaders();
    CreatePipelineStateObject();
    CreatePipelineStateObjectAnim();
}
void Renderer::RenderFrame(Model& mesh, float time, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up) {
    //Reset
    command_allocator_->Reset();
    command_list_->Reset(command_allocator_.Get(), pipeline_state_.Get());
    //command_list_->Reset(command_allocator_.Get(), pipeline_state_anim_.Get());
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
    //MSAA rt RESOLVE_SOURCE -> RENDER_TARGET
    if (!first_frame_) {
        barriersBegin[barrierCount++] = {
            D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            D3D12_RESOURCE_BARRIER_FLAG_NONE,
            {
                msaa_render_target_.Get(),
                D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
                D3D12_RESOURCE_STATE_RENDER_TARGET
            }
        };
    }
    command_list_->ResourceBarrier(barrierCount, barriersBegin);
    //Viewport / Scissor
    command_list_->RSSetViewports(1, &viewport_);
    command_list_->RSSetScissorRects(1, &scissor_rect_);
    //RTV / DSV (MSAA rt instead of back buffer)
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsv_heap_->GetCPUDescriptorHandleForHeapStart();
    command_list_->OMSetRenderTargets(1, &msaa_rtv_handle_, FALSE, &dsvHandle);
    //Clear MSAA rt and DSV
    const float clearColor[] = { 0.2f, 0.4f, 0.6f, 1.0f };
    command_list_->ClearRenderTargetView(msaa_rtv_handle_, clearColor, 0, nullptr);
    command_list_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    //PSO Root Signature
    command_list_->SetPipelineState(pipeline_state_.Get());
    command_list_->SetGraphicsRootSignature(root_signature_.Get());
    //Descriptor heaps
    ID3D12DescriptorHeap* heaps[] = {
        cbv_srv_uav_heap_.Get(),
        sampler_heap_.Get()
    };
    command_list_->SetDescriptorHeaps(_countof(heaps), heaps);
    //Root descriptor tables
    D3D12_GPU_DESCRIPTOR_HANDLE handle =
        cbv_srv_uav_heap_->GetGPUDescriptorHandleForHeapStart();

    //MVP CBV
    command_list_->SetGraphicsRootDescriptorTable(0, handle);
    handle.ptr += cbv_srv_uav_descriptor_size_;

    //Light CBV
    command_list_->SetGraphicsRootDescriptorTable(1, handle);
    handle.ptr += cbv_srv_uav_descriptor_size_;

    //Texture SRV
    command_list_->SetGraphicsRootDescriptorTable(2, handle);

    //Sampler
    command_list_->SetGraphicsRootDescriptorTable(
        3, sampler_heap_->GetGPUDescriptorHandleForHeapStart());

    //IA
    command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    command_list_->IASetVertexBuffers(0, 1, &vertex_buffer_view_);

    //Draw
    LightConstants lightData{};
    MVPConstants mvpData{};
    XMStoreFloat4x4(&mvpData.model, XMMatrixIdentity());
    // view
    XMStoreFloat4x4(&mvpData.view, XMMatrixLookAtLH(cam_pos, look_at, up));
    // projection
    XMStoreFloat4x4(&mvpData.projection, XMMatrixPerspectiveFovLH(XM_PIDIV4, float(width_) / float(height_), 0.1f, 1000.0f));
    for (const auto& submesh : mesh.GetSubMeshes()) {
        // Set the SRV for this submesh's material
        MaterialData material = mesh.GetMaterials()[submesh.materialIndex];
        lightData.ambient_k = XMFLOAT4(material.ambient_k.x, material.ambient_k.y, material.ambient_k.z, 1.0f);
        lightData.diffuse_k = XMFLOAT4(material.diffuse_k.x, material.diffuse_k.y, material.diffuse_k.z, 1.0f);
        lightData.specular_k = XMFLOAT4(material.specular_k.x, material.specular_k.y, material.specular_k.z, 1.0f);
        lightData.shiny_k = material.shiny_k;
        lightData.pad3[0] = lightData.pad3[1] = lightData.pad3[2] = 0.0f;
        memcpy(light_cb_mapped_, &lightData, sizeof(LightConstants));
        mvpData.time = time;
        memcpy(mvp_cb_mapped_, &mvpData, sizeof(MVPConstants));
        if (material.diffuseTexPath=="textures/sponza_thorn_diff.tga" or material.diffuseTexPath == "textures/vase_plant.tga")
        {
            command_list_->SetPipelineState(pipeline_state_anim_.Get());
        }else{
           command_list_->SetPipelineState(pipeline_state_.Get());
        }
        D3D12_GPU_DESCRIPTOR_HANDLE texHandle = cbv_srv_uav_heap_->GetGPUDescriptorHandleForHeapStart();
        texHandle.ptr += (2+ submesh.materialIndex)* cbv_srv_uav_descriptor_size_;
        command_list_->SetGraphicsRootDescriptorTable(2, texHandle);
        // Draw
        command_list_->DrawInstanced(
            static_cast<UINT>(submesh.vertexCount),
            1,
            static_cast<UINT>(submesh.startVertex),
            0
        );
    }
    //Resolve MSAA rt RENDER_TARGET-> RESOLVE_SOURCE
    D3D12_RESOURCE_BARRIER preResolveBarrier = {
        D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        D3D12_RESOURCE_BARRIER_FLAG_NONE,
        {
            msaa_render_target_.Get(),
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_RESOLVE_SOURCE
        }
    };
    command_list_->ResourceBarrier(1, &preResolveBarrier);
    // Resolve -> back buffer
    command_list_->ResolveSubresource(render_targets_[current_backbuffer_].Get(),0,msaa_render_target_.Get(),0, DXGI_FORMAT_R8G8B8A8_UNORM);
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

    if (fence_->GetCompletedValue() < fenceValue) {
        HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        fence_->SetEventOnCompletion(fenceValue, eventHandle);
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    first_frame_ = false;
}