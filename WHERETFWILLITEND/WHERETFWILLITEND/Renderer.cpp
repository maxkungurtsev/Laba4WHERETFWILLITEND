#include "Renderer.h"

void Renderer::CreateGraphicsDevice(UINT width, UINT height, int frame_count) {
    width_ = width;
    height_ = height;
    frame_count_= frame_count;
    HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create command queue");
    }
};

void Renderer::CreateFence() {
    HRESULT hr = device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create command queue");
    }
};

void Renderer::AskDescryptorSizes() {
    rtv_descriptor_size_ =device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    dsv_descriptor_size_ =device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    cbv_srv_uav_descriptor_size_ =device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    sampler_descriptor_size_ =device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
};

bool Renderer::check4XMSAA() {
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQuality{};
    msaaQuality.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    msaaQuality.SampleCount = 4;
    HRESULT hr = device_->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQuality, sizeof(msaaQuality));
    if (FAILED(hr)) {
        return false;
    }
    return msaaQuality.NumQualityLevels > 0;
};

void Renderer::CreateCommandStuff() {
    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;  // графические команды
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

void Renderer::CreateSwapChain(HWND hwnd)
{
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

void Renderer::CreateHeaps() {
    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.NumDescriptors = frame_count_;  // обычно 2 или 3
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
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    hr = device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&cbv_srv_uav_heap_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create CBV, SRV and UAV heap");
    }
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    hr = device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&sampler_heap_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create SAMPLER heap");
    }
};

void Renderer::CreateRTV() {
    render_targets_ = std::vector<ComPtr<ID3D12Resource>> (frame_count_);
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = rtv_heap_->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < frame_count_; i++){
        rtv_handle.ptr = SIZE_T(rtv_handle.ptr + i * rtv_descriptor_size_);
        HRESULT hr = swap_chain_->GetBuffer(i,IID_PPV_ARGS(&render_targets_[i]));
        if (FAILED(hr)){
            throw std::runtime_error("Failed to get swapchain buffer");
        }
        device_->CreateRenderTargetView(render_targets_[i].Get(),nullptr, rtv_handle);
    }
};

void Renderer::CreateZBuffer()
{
    //resourse desc
    D3D12_RESOURCE_DESC depthDesc{};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = width_;
    depthDesc.Height = height_;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    if (check4XMSAA()){
    depthDesc.SampleDesc.Count = 4;    
    }
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
    device_->CreateCommittedResource(&heapProps,D3D12_HEAP_FLAG_NONE,&depthDesc,D3D12_RESOURCE_STATE_DEPTH_WRITE,&optClear,IID_PPV_ARGS(&z_buffer_));
    //DSV
    device_->CreateDepthStencilView(z_buffer_.Get(), nullptr, dsv_heap_->GetCPUDescriptorHandleForHeapStart());
}

void Renderer::ViewportScissorSetup()
{
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

    command_list_->RSSetViewports(1, &viewport_);
    command_list_->RSSetScissorRects(1, &scissor_rect_);
}

void Renderer::CreateRootSignature() {
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = 0;
    rootSigDesc.pParameters = nullptr;
    rootSigDesc.NumStaticSamplers = 0;
    rootSigDesc.pStaticSamplers = nullptr;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    ComPtr<ID3DBlob> serializedRootSig;
    ComPtr<ID3DBlob> error;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc,D3D_ROOT_SIGNATURE_VERSION_1,&serializedRootSig,&error);
    if (FAILED(hr)) {
        if (error) {
            std::cerr << (char*)error->GetBufferPointer();
        }
        throw std::runtime_error("Failed to serialize root signature");
    }
    ComPtr<ID3D12RootSignature> root_signature_;
    device_->CreateRootSignature(0,serializedRootSig->GetBufferPointer(),serializedRootSig->GetBufferSize(),IID_PPV_ARGS(&root_signature_));
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
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;

    ComPtr<ID3D12PipelineState> pipeline_state_;
    device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipeline_state_));

};

void Renderer::CreateVertexBuffer(Model mesh) {

    std::vector<Vec3f> verts;
    std::vector<Vec2f> textures;
    std::vector<Vec3f> normals;
    for (int i = 0; i < mesh.Get_verts_amount(); i++) {
        verts.push_back(mesh.Get_vert(i));
    }
    for (int i = 0; i < mesh.Get_textures_amount(); i++) {
        textures.push_back(mesh.Get_vertex_texture(i));
    }
    for (int i = 0; i < mesh.Get_normals_amount(); i++) {
        normals.push_back(mesh.Get_normal(i));
    }
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = sizeof(verts);
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    device_->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertex_buffer_)
    );

    // Копируем данные в буфер
    void* pData;
    vertex_buffer_->Map(0, nullptr, &pData);
    memcpy(pData, verts.data(), verts.size()*sizeof(Vec3f));
    vertex_buffer_->Unmap(0, nullptr);

    // Настройка vertex buffer view
    vertex_buffer_view_.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
    vertex_buffer_view_.StrideInBytes = sizeof(verts[0]);
    vertex_buffer_view_.SizeInBytes = verts.size() * sizeof(Vec3f);
}

void Renderer::Initialize(UINT width, UINT height, int frame_count, HWND hwnd,Model mesh) {
    CreateGraphicsDevice(width, height, frame_count);
    CreateFence();
    AskDescryptorSizes();
    check4XMSAA();
    CreateCommandStuff();
    CreateSwapChain(hwnd);
    CreateHeaps();
    CreateRTV();
    CreateZBuffer();
    ViewportScissorSetup();
    CreateRootSignature();
    CreateVertexBuffer(mesh);
    command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    command_list_->IASetVertexBuffers(0, 1, &vertex_buffer_view_);
};

<<<<<<< HEAD
void Renderer::RenderFrame(){
    command_allocator_->Reset();
    command_list_->RSSetViewports(1, &viewport_);
    command_list_->RSSetScissorRects(1, &scissor_rect_);
    command_list_->Reset(command_allocator_.Get(), nullptr);
    command_list_->RSSetViewports(1, &viewport_);
    command_list_->RSSetScissorRects(1, &scissor_rect_);
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = render_targets_[current_backbuffer_].Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    command_list_->ResourceBarrier(1, &barrier);

    // 4. Set render target (RTV + DSV)
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = rtv_heap_->GetCPUDescriptorHandleForHeapStart();
    rtv_handle.ptr += current_backbuffer_ * rtv_descriptor_size_;
    D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle = dsv_heap_->GetCPUDescriptorHandleForHeapStart();
     command_list_->OMSetRenderTargets(1, &rtv_handle, FALSE, &dsv_handle);
    const float clearColor[] = { 0.2f, 0.4f, 0.6f, 1.0f };
    command_list_->ClearRenderTargetView(rtv_handle, clearColor, 0, nullptr);
    command_list_->ClearDepthStencilView(dsv_handle,D3D12_CLEAR_FLAG_DEPTH,1.0f, 0, 0, nullptr);
    std::swap(barrier.Transition.StateBefore, barrier.Transition.StateAfter);
    command_list_->ResourceBarrier(1, &barrier);
    command_list_->Close();
    ID3D12CommandList* lists[] = { 
        command_list_.Get() 
    };
    command_queue_->ExecuteCommandLists(1, lists);
    swap_chain_->Present(1, 0); 
    current_backbuffer_ = swap_chain_->GetCurrentBackBufferIndex();
}


=======
void Renderer::Renderframe() {
}
>>>>>>> parent of 4acdfb1 (R E N D E R   T I M E)
