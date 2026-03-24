#include "NEWRenderer.h"
void NewRenderer::CreateBackbuffer() {
    back_buffer_ = std::vector<ComPtr<ID3D12Resource>>(frame_count_);
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = device_->heaps_->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < frame_count_; i++) {
        rtv_handle.ptr += SIZE_T(i) * device_->heaps_->GetRTVHeapDescriptorSize();
        HRESULT hr = swap_chain_->GetBuffer(i, IID_PPV_ARGS(&back_buffer_[i]));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to get swapchain buffer");
        }
        device_->GetDXDevice()->CreateRenderTargetView(back_buffer_[i].Get(), nullptr, rtv_handle);
    }
};

void NewRenderer::CreateGeomRootSign(int textures_amount) {
    //cbv
    geom_root_signature_->AddParameter(Type::cbv, 1, D3D12_SHADER_VISIBILITY_ALL);
    // textures diffuse and normal
    geom_root_signature_->AddParameter(Type::srv, textures_amount, D3D12_SHADER_VISIBILITY_PIXEL);
    geom_root_signature_->AddParameter(Type::srv, textures_amount, D3D12_SHADER_VISIBILITY_PIXEL, 100);
    //sampler
    geom_root_signature_->AddParameter(Type::sampler, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    //creating root sign with said params
    geom_root_signature_->CreateRootSignature(device_);
};

void NewRenderer::CreateLightRootSign() {
    //cbv
    light_root_signature_->AddParameter(Type::cbv, 1, D3D12_SHADER_VISIBILITY_ALL);
    // g buffer
    light_root_signature_->AddParameter(Type::srv, 3, D3D12_SHADER_VISIBILITY_PIXEL);
    //sampler
    light_root_signature_->AddParameter(Type::sampler, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    //creating root sign with said params
    light_root_signature_->CreateRootSignature(device_);
};

void NewRenderer::FillCbuffer(XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, int time, XMFLOAT3 amb_light) {
    //identity
    XMStoreFloat4x4(&cbuffer_->GetData().model, XMMatrixIdentity());
    //view
    XMStoreFloat4x4(&cbuffer_->GetData().view, XMMatrixLookAtLH(cam_pos, look_at, up));
    //projection
    XMStoreFloat4x4(&cbuffer_->GetData().projection, XMMatrixPerspectiveFovLH(XM_PIDIV4, float(device_->width_) / float(device_->height_), 0.1f, 10000.0f));
    //inv identity
    XMStoreFloat4x4(&cbuffer_->GetData().inv_model, XMMatrixInverse(nullptr, XMLoadFloat4x4(&cbuffer_->GetData().model)));
    //inv view
    XMStoreFloat4x4(&cbuffer_->GetData().inv_view, XMMatrixInverse(nullptr, XMLoadFloat4x4(&cbuffer_->GetData().view)));
    //inv projection
    XMStoreFloat4x4(&cbuffer_->GetData().inv_projection, XMMatrixInverse(nullptr, XMLoadFloat4x4(&cbuffer_->GetData().projection)));
    cbuffer_->GetData().time = time;
    cbuffer_->GetData().amb_light = amb_light;
    XMStoreFloat4(&cbuffer_->GetData().cam_pos, cam_pos);
    XMStoreFloat4(&cbuffer_->GetData().cam_forward, look_at);
    LightData light;
    light.direction = {1.0f, 1.0f, 1.0f, 1.0f};
    light.falloff_end = 0.5;
    light.falloff_start = 0.3;
    light.position = {0.0f, 10.0f, 10.0f, 1.0f};
    light.spot_power = 10.0f;
    light.strength = {10.0f, 10.0f, 10.0f};
    light.type = 1;
    cbuffer_->GetData().lights[0]=light;
    cbuffer_->GetData().max_lights = 1;
    for (int i = 0; i < mesh_->GetMaterials().size(); i++) {
        int index = min(i, 128);
        cbuffer_->GetData().mats[index].ambient_ = mesh_->GetMaterials()[i].ambient_k;
        cbuffer_->GetData().mats[index].diffuse_ = mesh_->GetMaterials()[i].diffuse_k;
        cbuffer_->GetData().mats[index].spec_ = mesh_->GetMaterials()[i].specular_k;
        cbuffer_->GetData().mats[index].shiny_ = mesh_->GetMaterials()[i].shiny_k;
    }
    cbuffer_->Save_changes();
};

void NewRenderer::CreateInputLayout() {
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

void NewRenderer::CreateVertexBuffer(std::shared_ptr<Model> model) {
    const std::vector<Vertex>& vertices = model->GetVertices();
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
    HRESULT hr = device_->GetDXDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertex_buffer_));
    if (FAILED(hr))
        throw std::runtime_error("Failed to create vertex buffer");
    void* mappedData = nullptr;
    D3D12_RANGE readRange{ 0, 0 };
    hr = vertex_buffer_->Map(0, &readRange, &mappedData);
    if (FAILED(hr))
        throw std::runtime_error("Failed to fill vertex buffer");
    memcpy(mappedData, vertices.data(), bufferSize);
    vertex_buffer_->Unmap(0, nullptr);
    vertex_buffer_view_.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
    vertex_buffer_view_.StrideInBytes = sizeof(Vertex);
    vertex_buffer_view_.SizeInBytes = bufferSize;
}

void NewRenderer::CompileShader(std::wstring path, ComPtr<ID3DBlob> shader) {
    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(path.c_str(), nullptr, nullptr, "main", "vs_5_0", 0, 0, &shader, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "Shader compile error: ";
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        else {
            OutputDebugStringA("Shader compile failed, but no error message was produced.");
        }
        throw std::runtime_error("Failed to compile shader");
    }
};

NewRenderer::NewRenderer(UINT width, UINT height, int frame_count, Window* hwnd, std::string mesh_path, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, int time) {
    frame_count_ = frame_count;
	//device, cmd, fence, heaps, viewport, scissor
	device_ = std::make_shared<Gdevice>(width, height, 100+frame_count);
    swap_chain_=hwnd->CreateSwapChain(device_);
    mesh_ = std::make_shared<Model>(mesh_path, device_);
    CreateBackbuffer();
	//g buffer
    g_buffer_ = std::make_shared<GBuffer>(width, height, device_);
    Sampler_handle_ = device_->heaps_->MakeSampler();
    // make root signs
    CreateGeomRootSign(mesh_->GetMaterials().size());
    CreateLightRootSign();
    CreateInputLayout();
    cbuffer_ = std::make_shared<Cbuffer<PassConstants>>(device_);
    FillCbuffer(cam_pos, look_at, up, time);
    CreateVertexBuffer(mesh_);
    CompileShader(L"VertexShader.hlsl",vertex_shader_);
    CompileShader(L"PixelShader.hlsl", pixel_shader_);
    pso_ = std::make_shared<PSO>(input_layout_, vertex_shader_, pixel_shader_, device_, geom_root_signature_);
}
void NewRenderer::RenderFrame() {
    device_->cmd_->ResetAllocator();
    D3D12_RESOURCE_BARRIER barriersBegin[2];
    UINT barrierCount = 0;
    //back buffer: PRESENT -> RESOLVE_DEST
    barriersBegin[barrierCount++] = {
        D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        D3D12_RESOURCE_BARRIER_FLAG_NONE,
        {
            back_buffer_[current_backbuffer_].Get(),
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RESOLVE_DEST
        }
    };
    device_->cmd_->command_list_->ResourceBarrier(barrierCount, barriersBegin);
    //Viewport / Scissor
    device_->cmd_->command_list_->RSSetViewports(1, &device_->viewport_);
    device_->cmd_->command_list_->RSSetScissorRects(1, &device_->scissor_rect_);
    //dsv
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = g_buffer_->depth_->handle_.cpu_;
    //command_list_->OMSetRenderTargets(1, &msaa_rtv_handle_, FALSE, &dsvHandle);
    const float clearColor[] = { 0.2f, 0.4f, 0.6f, 1.0f };
    device_->cmd_->command_list_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    device_->cmd_->command_list_->SetPipelineState(pso_->GetPSO().Get());
    device_->cmd_->command_list_->SetGraphicsRootSignature(geom_root_signature_->GetRootSign().Get());
    // heaps shit
    ID3D12DescriptorHeap* heaps[] = {
        device_->heaps_->GetCBV_SRV_UAV_Heap().Get(),
        device_->heaps_->GetSamplerHeap().Get()
    };
    device_->cmd_->command_list_->SetDescriptorHeaps(_countof(heaps), heaps);
    //set root params
    //cbv
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(0, cbuffer_->GetHandle().gpu_);
    //diffuse textures
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(1, mesh_->GetMaterials()[0].diffuseTexture->GetResourse()->GetHandle().gpu_);
    //normal textures
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(2, mesh_->GetMaterials()[0].NormalTexture->GetResourse()->GetHandle().gpu_);
    //sampler
    device_->cmd_->command_list_->SetGraphicsRootDescriptorTable(3, Sampler_handle_.gpu_);

    device_->cmd_->command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_->cmd_->command_list_->IASetVertexBuffers(0, 1, &vertex_buffer_view_);


    //DRAW
    cbuffer_->GetData().current_mat = 0;
    for (const auto& submesh : mesh_->GetSubMeshes()) {
        device_->cmd_->command_list_->DrawInstanced(
            static_cast<UINT>(submesh.vertexCount),
            1,
            static_cast<UINT>(submesh.startVertex),
            0
        );
        cbuffer_->GetData().current_mat += 1;
        cbuffer_->Save_changes();
    } 

    device_->cmd_->command_list_->Close();
    ID3D12CommandList* lists[] = { device_->cmd_->command_list_.Get() };
    device_->cmd_->command_queue_->ExecuteCommandLists(1, lists);

    //Present
    swap_chain_->Present(1, 0);
    current_backbuffer_ = swap_chain_->GetCurrentBackBufferIndex();

    //Fence
    device_->fence_->IncrementFenceValue();
    device_->cmd_->command_queue_->Signal(device_->fence_->GetFence().Get(), device_->fence_->GetFenceValue());

    if (device_->fence_->GetFence()->GetCompletedValue() < device_->fence_->GetFenceValue()) {
        HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        device_->fence_->GetFence()->SetEventOnCompletion(device_->fence_->GetFenceValue(), eventHandle);
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}